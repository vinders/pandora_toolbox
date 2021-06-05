/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include <stdexcept>

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class RasterizerState
        /// @brief Configured rasterizer state resource - can be used with Renderer.setRasterizerState
        class RasterizerState final {
        public:
          RasterizerState() = default;
          RasterizerState(void* state) : _state(state) {}
          RasterizerState(const RasterizerState&) = delete;
          RasterizerState(RasterizerState&& rhs) noexcept : _state(rhs._state) { rhs._state = nullptr; }
          RasterizerState& operator=(const RasterizerState&) = delete;
          RasterizerState& operator=(RasterizerState&& rhs) noexcept { this->_state = rhs._state; rhs._state = nullptr; return *this; }
          ~RasterizerState() noexcept;
          
          inline operator bool() const noexcept { return (this->_state != nullptr); }
          inline bool isValid() const noexcept { return (this->_state != nullptr); } ///< Container has a value
          inline void* get() const noexcept { return this->_state; } ///< Get container value
          
        private:
          void* _state = nullptr; // ID3D11RasterizerState*
        };
        
        // ---
        
        /// @class FilterStates
        /// @brief Array of configured sampler filter state resources - can be used with Renderer.setFilterStates
        class FilterStates final {
        public:
          using State = void*;
          
          FilterStates();
          FilterStates(const FilterStates&) = delete;
          FilterStates(FilterStates&& rhs) noexcept 
            : _states(std::move(rhs._states)), _length(rhs._length) { rhs._states = nullptr; rhs._length = 0; }
          FilterStates& operator=(const FilterStates&) = delete;
          FilterStates& operator=(FilterStates&& rhs) noexcept { 
            this->_states = std::move(rhs._states); 
            this->_length = rhs._length; 
            rhs._states = nullptr; rhs._length = 0;
            return *this; 
          }
          ~FilterStates() noexcept;
          
          inline operator bool() const noexcept { return (this->_length != 0); }
          inline bool empty() const noexcept { return (this->_length == 0); } ///< Container is empty
          inline size_t size() const noexcept { return this->_length; } ///< Number of values in container
          static size_t maxSize() noexcept; ///< Max number of values in container
          
          inline bool append(State state) noexcept { 
            if (size() < maxSize()) {
              this->_states[this->_length] = state; 
              ++(this->_length);
              return true;
            }
            return false;
          }
          bool insert(uint32_t index, State state) noexcept;
          void erase(uint32_t index) noexcept;
          void clear() noexcept;
          
          inline State* get() noexcept { return this->_states; }
          inline const State* get() const noexcept { return this->_states; }
          
          inline State at(uint32_t index) const {
            if (index >= this->_length)
              throw std::out_of_range("FilterState: index out of range");
            return this->_states[index];
          }
          
        private:
          State* _states = nullptr; // <ID3D11SamplerState*>
          size_t _length = 0;
        };
      }
    }
  }
  
#endif
