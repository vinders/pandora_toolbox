/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <utility>
#include <type_traits>
#include <system/trace.h>

namespace pandora {
  namespace thread {
    /// @brief Task processing function usage for thread pool
    enum class ThreadRunnerMode : uint32_t {
      single = 0, ///< One common function, passed on construction and called for every job (more efficient)
      perJob = 1  ///< A different function can be provided for each job (instead of default function, if provided)
    };
    /// @brief Type of task processing function in thread pool
    enum class TaskRunnerType : uint32_t {
      functionPointer = 0, ///< C pointer to task processing function
      lambda = 1           ///< std::function or lambda
    };

    /// ---

    /// @class ThreadPool
    /// @brief Fixed-size pool of threads, ready to process some asynchronous jobs on demand.
    /// @description Simple pool of threads with a pre-defined size.
    ///              The threads are ready to process some jobs asynchronously, without needing to be created (they already exist).
    ///              The tasks will be started by any thread in the pool in the order of their arrival.
    template <typename _JobParamType,                        // Type of data container to process (must be movable)
              ThreadRunnerMode _Mode = ThreadRunnerMode::single, // one function passed on construction / one per job
              TaskRunnerType _FunctionType = TaskRunnerType::functionPointer> // function pointer / lambda
    class ThreadPool final {
    public:
      using Type = ThreadPool<_JobParamType,_Mode,_FunctionType>;
      using size_type = size_t;
      using task_runner_type = typename std::conditional<_FunctionType == TaskRunnerType::lambda, std::function<void(_JobParamType&)>, void (*)(_JobParamType&)>::type;
      using task_runner_move = typename std::conditional<_FunctionType == TaskRunnerType::lambda, task_runner_type&&, task_runner_type>::type;
    protected:
      template<bool cond, typename _T>
      using EnableIf = typename std::enable_if<cond, _T>::type;

      using job_param_move = typename std::conditional<std::is_class<_JobParamType>::value, _JobParamType&&, _JobParamType>::type;
      struct JobParamWithRunner { // Job data in 'perJob' mode (parameter + custom runner)
        _JobParamType param;
        task_runner_type runner;
        JobParamWithRunner(job_param_move param) : param(std::move(param)), runner(nullptr) {}
        JobParamWithRunner(job_param_move param, task_runner_move runner) : param(std::move(param)), runner(std::move(runner)) {}
      };
      using job_item = typename std::conditional<(_Mode == ThreadRunnerMode::perJob), JobParamWithRunner, _JobParamType>::type;

      struct SharedPoolData { // Synchronization & jobs data - shared with child threads
        bool isRunning = false;
        size_t busyThreads = 0u;
        std::queue<job_item> jobs;
        mutable std::mutex lock;
        std::condition_variable condition;
      };

    public:
      /// @brief Create empty thread pool - not running
      inline ThreadPool() : _poolData(std::make_shared<SharedPoolData>()) {}
      /// @brief Create and start a thread pool - mode: no common runner -> provide a task runner for each job 
      /// @warning in 'single' runner mode, jobs will not be processed!
      inline ThreadPool(size_t threadCount) : _poolData(std::make_shared<SharedPoolData>()) {
        task_runner_type emptyRunner = nullptr; 
        _startThreads(threadCount, emptyRunner);
      }
      /// @brief Create and start a thread pool - mode: common task runner provided in constructor 
      /// @warning Required for standard use of 'single' runner mode, optional to have a default runner in 'perJob' mode.
      inline ThreadPool(size_t threadCount, task_runner_type commonRunner) : _poolData(std::make_shared<SharedPoolData>()) {
        _startThreads(threadCount, commonRunner);
      }

      /// @brief Stop thread pool and wait for each thread to stop running
      ~ThreadPool() noexcept { _stopThreads(); }

      ThreadPool(const Type&) = delete;
      Type& operator=(const Type&) = delete;
      inline ThreadPool(Type&& rhs) noexcept : _poolData(std::move(rhs._poolData)) {
        std::swap(this->_threads, rhs._threads);
        rhs._poolData = std::make_shared<SharedPoolData>();
      }
      inline Type& operator=(Type&& rhs) noexcept {
        _stopThreads();
        std::swap(this->_threads, rhs._threads);
        this->_poolData = std::move(rhs._poolData);
        rhs._poolData = std::make_shared<SharedPoolData>();
        return *this;
      }

      // -- thread pool size --

      inline constexpr size_type size() const noexcept { return this->_threads.size(); } ///< Total number of threads in the pool
      /// @brief Number of active threads in the pool (currently processing a job)
      inline size_type busyThreads() const noexcept { std::lock_guard<std::mutex> guard(this->_poolData->lock); return this->_poolData->busyThreads; }
      /// @brief Number of threads waiting in the pool (ready for new jobs)
      inline size_type freeThreads() const noexcept { std::lock_guard<std::mutex> guard(this->_poolData->lock); return size() - this->_poolData->busyThreads; }

      // -- job management --

      /// @brief Insert a new job to process (copied) - custom task runner for each job (not available in 'single' runner mode)
      template<typename J = _JobParamType, ThreadRunnerMode M = _Mode>
      inline EnableIf<(!std::is_class<J>::value || std::is_copy_constructible<J>::value) && M == ThreadRunnerMode::perJob,
                      bool> addJob(const _JobParamType& param, task_runner_move runner) {
        std::unique_lock<std::mutex> guard(this->_poolData->lock);
        bool isSuccess = (this->_poolData->isRunning);
        if (isSuccess)
          this->_poolData->jobs.emplace(_JobParamType(param), std::move(runner));
        guard.unlock();

        this->_poolData->condition.notify_one();
        return isSuccess;
      }
      /// @brief Insert a new job to process (moved) - custom task runner for each job (not available in 'single' runner mode)
      template<ThreadRunnerMode M = _Mode>
      inline EnableIf<M==ThreadRunnerMode::perJob, 
                      bool> addJob(_JobParamType&& param, task_runner_move runner) {
        std::unique_lock<std::mutex> guard(this->_poolData->lock);
        bool isSuccess = (this->_poolData->isRunning);
        if (isSuccess)
          this->_poolData->jobs.emplace(std::move(param), std::move(runner));
        guard.unlock();

        this->_poolData->condition.notify_one();
        return isSuccess;
      }

      /// @brief Insert a new job to process (copied) - use common task runner provided in constructor
      template<typename J = _JobParamType>
      inline EnableIf<!std::is_class<J>::value || std::is_copy_constructible<J>::value, 
                      bool> addJob(const _JobParamType& param) {
        std::unique_lock<std::mutex> guard(this->_poolData->lock);
        bool isSuccess = (this->_poolData->isRunning);
        if (isSuccess)
          this->_poolData->jobs.emplace(_JobParamType(param));
        guard.unlock();

        this->_poolData->condition.notify_one();
        return isSuccess;
      }
      /// @brief Insert a new job to process (moved) - use common task runner provided in constructor
      inline bool addJob(_JobParamType&& param) {
        std::unique_lock<std::mutex> guard(this->_poolData->lock);
        bool isSuccess = (this->_poolData->isRunning);
        if (isSuccess)
          this->_poolData->jobs.emplace(std::move(param));
        guard.unlock();

        this->_poolData->condition.notify_one();
        return isSuccess;
      }

      /// @brief Cancel all jobs that haven't already been started
      inline uint32_t cancelPendingJobs() noexcept {
        std::lock_guard<std::mutex> guard(this->_poolData->lock);
        size_t jobCount = this->_poolData->jobs.size();
        this->_poolData->jobs = std::queue<job_item>{};
        return static_cast<uint32_t>(jobCount);
      }

    protected:
      // -- thread management --

      // launch thread pool
      void _startThreads(size_t threadCount, task_runner_type& runner) {
        this->_poolData->isRunning = true;
        this->_poolData->busyThreads = 0u;
        this->_threads.reserve(threadCount);
        for (uint32_t index = 0; index < threadCount; ++index)
          this->_threads.emplace_back(&Type::_runThread, this->_poolData, index, runner);
      }

      // stop all threads
      void _stopThreads() noexcept {
        std::unique_lock<std::mutex> guard(this->_poolData->lock);
        this->_poolData->isRunning = false;
        this->_poolData->jobs = std::queue<job_item>{};
        guard.unlock();

        this->_poolData->condition.notify_all();
        for (auto& item : this->_threads) {
          if (item.joinable()) {
            try { 
              item.join(); 
            }
            catch (const std::exception& __DEBUG_ARG__(exc)) {
              TRACE_N("ThreadPool: thread join exception: %s", exc.what());
              try { item.detach(); } catch (const std::exception&) {}
            }
          }
        }
        this->_threads.clear();
      }

      // -- thread execution --

      // main thread execution loop
      static void _runThread(std::shared_ptr<SharedPoolData> shared, uint32_t __DEBUG_ARG__(index), task_runner_type commonRunner) noexcept {
        TRACE_N("ThreadPool: thread %u started", index);
        assert(shared != nullptr);
        SharedPoolData& sync = *shared;
        std::queue<job_item>& jobs = shared->jobs;
        if (commonRunner == nullptr)
          commonRunner = &Type::_defaultRunner;

        std::unique_lock<std::mutex> guard(sync.lock);
        while (sync.isRunning) {
          while (sync.isRunning && jobs.empty())
            sync.condition.wait(guard);

          if (sync.isRunning) {
            TRACE_N("ThreadPool: thread %u - job started", index);
            ++(sync.busyThreads);
            job_item jobData = std::move(jobs.front());
            jobs.pop();
            guard.unlock();

            try {
              _callRunner(jobData, commonRunner);
            }
            catch (const std::exception& __DEBUG_ARG__(exc)) { TRACE_N("ThreadPool: exception: %s", exc.what()); }
            catch (...) { TRACE("ThreadPool: unknown exception type thrown"); }

            guard.lock();
            --(sync.busyThreads);
          }
        }

        guard.unlock();
        sync.condition.notify_one();
        TRACE_N("ThreadPool: thread %u stopped", index);
      }

      // specialized calls to task runner
      static inline void _callRunner(JobParamWithRunner& jobData, task_runner_type& defaultRunner) {
        if (jobData.runner != nullptr)
          jobData.runner(jobData.param);
        else
          defaultRunner(jobData.param);
      }
      static inline void _callRunner(_JobParamType& param, task_runner_type& defaultRunner) {
        defaultRunner(param);
      }
      // default task runner, if none provided
      static inline void _defaultRunner(_JobParamType&) {
        TRACE("ThreadPool: no common runner provided.");
      }

    private:
      std::vector<std::thread> _threads;
      std::shared_ptr<SharedPoolData> _poolData;
    };

  }
}
