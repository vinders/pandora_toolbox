/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include <cstring>
# include <stdexcept>

# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
# include "video/d3d11/api/d3d_11.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class RendererState
        /// @brief Rendererer state resource template - rasterizer settings, sampler filter, depth/stencil state, blend state...
        template <typename _Type>
        class RendererState final {
        public:
          RendererState() = default; ///< Create empty state
          RendererState(_Type* state) : _state(state) {} ///< Initialize state
          RendererState(const RendererState&) = delete;
          RendererState(RendererState&& rhs) noexcept : _state(rhs._state) { rhs._state = nullptr; }
          RendererState& operator=(const RendererState&) = delete;
          RendererState& operator=(RendererState&& rhs) noexcept { this->_state = rhs._state; rhs._state = nullptr; return *this; }
          ~RendererState() noexcept { release(); }
          
          inline void release() noexcept { ///< Destroy state resource
            if (this->_state) {
              try { this->_state->Release(); } catch (...) {}
              this->_state = nullptr;
            }
          }
          inline _Type* extract() noexcept { ///< Return state resource and release ownership (to store in array, for example)
            _Type* state = this->_state;
            this->_state = nullptr;
            return state;
          }
          
          inline operator bool() const noexcept { return (this->_state != nullptr); } ///< Container has a value (not empty/moved/released)
          inline bool isValid() const noexcept { return (this->_state != nullptr); }  ///< Container has a value (not empty/moved/released)
          inline _Type* get() const noexcept { return this->_state; } ///< Get container value (or NULL if empty/moved/released)
          
        private:
          _Type* _state = nullptr;
        };
        
        /// @brief Configured depth/stencil state resource - can be used with Renderer.setDepthStencilState
        using DepthStencilState = RendererState<ID3D11DepthStencilState>;
        /// @brief Configured rasterizer state resource - can be used with Renderer.setRasterizerState
        using RasterizerState = RendererState<ID3D11RasterizerState>;
        /// @brief Configured blend state resource - can be used with Renderer.setBlendState
        using BlendState = RendererState<ID3D11BlendState>;
        /// @brief Configured sampler filter state resource - can be used with Renderer.set<...>FilterState
        using FilterState = RendererState<ID3D11SamplerState>;
        
        
        // ---
        
        /// @class RendererStateArray
        /// @brief Array of renderer state resources - store multiple states - set multiple states at once
        template <typename _Type, size_t _MaxSize>
        class RendererStateArray final {
        public:
          using State = _Type*;

          RendererStateArray() { ///< Create container for state resources
            this->_states = (_Type**)calloc(_MaxSize, sizeof(_Type*));
            if (this->_states == nullptr)
              throw std::bad_alloc();
          }
          
          RendererStateArray(const RendererStateArray&) = delete;
          RendererStateArray(RendererStateArray&& rhs) noexcept 
            : _states(std::move(rhs._states)), _length(rhs._length) { rhs._states = nullptr; rhs._length = 0; }
          RendererStateArray& operator=(const RendererStateArray&) = delete;
          RendererStateArray& operator=(RendererStateArray&& rhs) noexcept { 
            this->_states = std::move(rhs._states); this->_length = rhs._length; rhs._states = nullptr; rhs._length = 0;
            return *this; 
          }
          ~RendererStateArray() noexcept { ///< Destroy state resources and container
            if (this->_states) {
              clear();
              free(this->_states);
            }
          }
          
          // -- accessors --
          
          inline operator bool() const noexcept { return (this->_length != 0); } ///< Container is not empty
          inline bool empty() const noexcept { return (this->_length == 0); }    ///< Container is empty
          inline size_t size() const noexcept { return this->_length; } ///< Number of states currently in container
          static size_t maxSize() noexcept { return _MaxSize; } ///< Max number of states in container
          
          inline _Type** get() noexcept { return this->_states; } ///< Get entire collection, to bind all (or first) states to Renderer instance
          inline const _Type** get() const noexcept { return this->_states; }  ///< Get entire constant collection

          inline _Type* at(uint32_t index) const { ///< Get state located at index
            if (index >= this->_length)
              throw std::out_of_range("RendererStateArray.at: index out of range");
            return this->_states[index];
          }
          
          // -- operations --
          
          /// @brief Try to append new state resource (if max size not reached)
          inline bool append(RendererState<_Type>&& state) noexcept {
            if (size() < maxSize()) {
              this->_states[this->_length] = state.extract(); 
              ++(this->_length);
              return true;
            }
            return false;
          }
          /// @brief Try to insert new state resource at index (if max size not reached and index <= length)
          bool insert(uint32_t index, RendererState<_Type>&& state) noexcept {
            if (this->_length >= maxSize() || index > (uint32_t)this->_length)
              return false;
            
            if (index < (uint32_t)this->_length) {
              auto it = &(this->_states[index]);
              memmove((void*)(it + 1), (void*)it, (this->_length - (size_t)index)*sizeof(_Type*));
              *it = state.extract();
            }
            else
              this->_states[this->_length] = state.extract();
            ++(this->_length);
            return true;
          }
          
          /// @brief Destroy state resource at index (and shift greater indices)
          void erase(uint32_t index) noexcept {
            if (index < this->_length) {
              auto it = &(this->_states[index]);
              if (*it != nullptr) {
                try { (*it)->Release(); } catch (...) {}
              }
              if (index + 1 < (uint32_t)this->_length)
                memmove((void*)it, (void*)(it + 1), (this->_length - (size_t)index - 1u)*sizeof(_Type*));
              
              --(this->_length);
              this->_states[this->_length] = nullptr;
            }
          }
          /// @brief Destroy last state resource (if any)
          void pop_back() noexcept {
            if (this->_length > size_t{0}) {
              auto it = &(this->_states[this->_length - 1u]);
              if (*it != nullptr) {
                try { (*it)->Release(); } catch (...) {}
                *it = nullptr;
              }
              --(this->_length);
            }
          }
          /// @brief Destroy all state resources (container can still be used)
          void clear() noexcept {
            if (this->_length > size_t{ 0 }) {
              for (auto it = &this->_states[this->_length - 1u]; it >= this->_states; --it) {
                if (*it != nullptr) {
                  try { (*it)->Release(); } catch (...) {}
                  *it = nullptr;
                }
              }
              this->_length = 0;
            }
          }

        private:
          _Type** _states = nullptr;
          size_t _length = 0;
        };
        
        /// @brief Array of configured sampler filter state resources - can be used with Renderer.set<...>FilterStates
        using FilterStateArray = RendererStateArray<ID3D11SamplerState, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT>;
        /// @brief Array of configured blend state resources
        template <size_t _MaxSize>
        using DepthStencilStateArray = RendererStateArray<ID3D11DepthStencilState, _MaxSize>;
        /// @brief Array of configured blend state resources
        template <size_t _MaxSize>
        using RasterizerStateArray = RendererStateArray<ID3D11RasterizerState, _MaxSize>;
        /// @brief Array of configured blend state resources
        template <size_t _MaxSize>
        using BlendStateArray = RendererStateArray<ID3D11BlendState, _MaxSize>;
      }
    }
  }
  
# undef NODRAWTEXT
# undef NOGDI
# undef NOBITMAP
# undef NOMCX
# undef NOSERVICE
#endif
