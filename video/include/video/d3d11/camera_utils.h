/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include <system/align.h>

# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
# include <DirectXMath.h>

  namespace pandora {
    namespace video {
      namespace d3d11 {
        using MatrixFloat4x4 = DirectX::XMMATRIX; ///< 4x4 floating-point matrix (requires 16-bit memory alignment)

        // -- camera projection --
        
        /// @class Camera
        /// @brief Direct3D virtual camera - screen projection/limits
        /// @remarks - The same projection may be used with different camera view matrices (or none, if the camera never moves).
        ///          - On screen resize, the projection ratio must be updated (unless the window forces homothety).
        ///          - When often toggling between different projections (example: first-person / third-person), keeping multiple CameraProjection instances may be useful.
        /// @warning - XMMATRIX requires 16-bit memory alignment:
        ///            -> either store CameraProjection in stack variable or global variable;
        ///            -> either use a structure declared with __align_type(16, ...): see <system/align.h>;
        ///            -> either use dynamic allocation with _aligned_malloc / placement new.
        ///          - Most 64-bit compilers use 16-byte alignment by default, but 32-bit compilers do not -> be careful.
        __align_prefix(16)
        class CameraProjection final {
        public:
          /// @brief Create default camera projection (ratio 4:3, field-of-view 60°)
          CameraProjection() noexcept { _computeProjection(); }
          /// @brief Create camera projection
          /// @param displayWidth  Horizontal display size - used to calculate aspect ratio (no need to update after homothetic resize)
          /// @param displayHeight Vertical display size - used to calculate aspect ratio (no need to update after homothetic resize)
          /// @param fieldOfView   Camera field-of-view: angle between 1 and 180 degrees:
          ///                      * Usually 60 for cinematics and third-person views;
          ///                      * 90 or more for first-person views.
          /// @param nearPlane     Closest visible plane (shouldn't be too close, to avoid screen obstruction and z-buffer overload)
          /// @param farPlane      Furthest visible plane (shouldn't be infinite, to avoid reaching floating-point limits)
          CameraProjection(uint32_t displayWidth, uint32_t displayHeight, float fieldOfView = 60.0f, 
                           float nearPlane = 0.25f, float farPlane = 100.0f) noexcept
            : _displayRatio((float)displayWidth/(float)displayHeight),
              _fieldOfView(fieldOfView), _nearPlane(nearPlane), _farPlane(farPlane) { _computeProjection(); }
          
          CameraProjection(const CameraProjection&) = default;
          CameraProjection(CameraProjection&&) noexcept = default;
          CameraProjection& operator=(const CameraProjection&) = default;
          CameraProjection& operator=(CameraProjection&&) noexcept = default;
          ~CameraProjection() noexcept = default;
          
          // -- accessors --
          
          //TODO right-hand à la place ?
          //TODO draw test: ne pas transposer projection matrix, et utiliser coord Z négative pour vérifier si ça marche
          
          /// @brief Get projection matrix (may need to be transposed, depending on left-hand/right-hand shader conventions)
          /// @remarks - Should be stored in constant buffer for vertex shader.
          ///          - Transformation in shader: (vertex coord * world matrix per mesh * camera view matrix * projection matrix).
          ///          - Pre-combining on CPU: modelView = camera view matrix * world matrix; use separate projection matrix (for lighting).
          /// @warning - XMMATRIX requires 16-bit memory alignment:
          ///            -> either store CameraProjection in stack variable or global variable;
          ///            -> either use a structure declared with __align_type(16, ...): see <system/align.h>;
          ///            -> either use dynamic allocation with _aligned_malloc / placement new.
          ///          - Most 64-bit compilers use 16-byte alignment by default, but 32-bit compilers do not -> be careful.
          ///          - Another possibility is to store the result in a DirectX::XMFLOAT4 type with DirectX::XMStoreFloat4x4(...).
          inline MatrixFloat4x4 projectionMatrix() const noexcept { return this->_projection; }
          
          inline float displayRatio() const noexcept { return this->_displayRatio; } ///< Get current display aspect ratio
          inline float fieldOfView() const noexcept { return this->_fieldOfView; } ///< Get current field-of-view
          inline float nearPlane() const noexcept { return this->_nearPlane; } ///< Get current projection near-plane limit
          inline float farPlane() const noexcept { return this->_farPlane; } ///< Get current projection far-plane limit
          
          // -- updates --
          
          /// @brief Change camera projection on screen (screen ratio / field-of-view)
          inline void setProjection(uint32_t displayWidth, uint32_t displayHeight, float fieldOfView, 
                                    float nearPlane = 0.25f, float farPlane = 100.0f) noexcept {
            this->_displayRatio = (float)displayWidth/(float)displayHeight;
            this->_fieldOfView = fieldOfView;
            this->_nearPlane = nearPlane;
            this->_farPlane = farPlane;
            _computeProjection();
          }
          /// @brief Change screen aspect ratio (after non-homothetic resize)
          inline void setRatio(uint32_t screenWidth, uint32_t screenHeight) noexcept {
            this->_displayRatio = (float)screenWidth/(float)screenHeight;
            _computeProjection();
          }
          /// @brief Change field-of-view (zoom adjustment, switching between first/third-person view...)
          inline void setFieldOfView(float fieldOfView, float nearPlane = 0.25f, float farPlane = 100.0f) noexcept {
            this->_fieldOfView = fieldOfView;
            this->_nearPlane = nearPlane;
            this->_farPlane = farPlane;
            _computeProjection();
          }
          
          /// @brief Relative update of field-of-view (added to current value)
          inline void addFieldOfView(float increment) noexcept {
            this->_fieldOfView += increment;
            _computeProjection();
          }
          
        private:
          inline void _constrainFieldOfView() noexcept {
            if (this->_fieldOfView < 1.f)        this->_fieldOfView = 1.f;
            else if (this->_fieldOfView > 180.f) this->_fieldOfView = 180.f;
          }
          void _computeProjection() noexcept;
          
        private:
          MatrixFloat4x4 _projection;
          float _displayRatio = 4.0f/3.0f;
          float _fieldOfView = 60.0f;
          float _nearPlane = 0.25f;
          float _farPlane = 100.0f;
        } __align_suffix(16);


        // -- camera view/world helpers --

        class CameraUtils final {
        public:
          CameraUtils() = delete;

          static inline float toRadians(float degrees) noexcept { return DirectX::XMConvertToRadians(degrees); } ///< Degrees to radians
          static inline float toDegrees(float radians) noexcept { return DirectX::XMConvertToDegrees(radians); } ///< Radians to degrees

          static constexpr inline float maxPitchDegrees() noexcept { return 89.f; }     ///< Max value for pitch angle (to avoid reversal) - degrees
          static constexpr inline float maxPitchRadians() noexcept { return 1.55335f; } ///< Max value for pitch angle (to avoid reversal) - radians

          // -- matrix transformation --

          /// @brief Compute world translation/rotation matrix for a model (single-axis rotation)
          static MatrixFloat4x4 computeWorldMatrix(float modelPosition[3], float yaw) noexcept;

          /// @brief Transpose matrix (to invert order of factors when multiplying)
          static inline MatrixFloat4x4 transposeMatrix(MatrixFloat4x4& source) noexcept { 
            return DirectX::XMMatrixTranspose(source); 
          }

          /*/// @brief Create camera view matrix looking at a focal-point (not transposed)
          /// @remarks Useful for third-person views and for rotating/show-room cameras.
          static inline MatrixFloat4x4 lookAtViewMatrix(const Position& eyePosition, const Position& focalPoint, const Position& originUp) noexcept {
            return DirectX::XMMatrixLookAtLH(eyePosition, focalPoint, originUp);
          }
          /// @brief Create camera view matrix from eye position/direction (not transposed)
          /// @remarks Useful for first-person views implemented with direction unit vectors (eyeFront/eyeRight/eyeUp, for example).
          static inline MatrixFloat4x4 lookToViewMatrix(const Position& eyePosition, const Position& frontEyeDirection, const Position& originUp) noexcept {
            return DirectX::XMMatrixLookToLH(eyePosition, frontEyeDirection, originUp);
          }*/

          //roadmap:
          // - right hand / left hand
          // - quaternion/rotate
          // - lookAt/lookTo/rotateAround
        };
      }
    }
  }

# undef NODRAWTEXT
# undef NOGDI
# undef NOBITMAP
# undef NOMCX
# undef NOSERVICE
#endif
