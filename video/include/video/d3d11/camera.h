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
        using Position = DirectX::XMFLOAT3; ///< left-hand position: x=right / y=up / z=into
        using Angles = DirectX::XMFLOAT3; ///< Angles: x=yaw (left->right) / y=pitch (down->up) / z=roll (tilt clockwise)
        
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
        
        
        // -- camera view helpers --
        
        //TODO right-hand ???
        
        /// @class CameraViewUtils
        /// @brief Utilities to help implement camera views
        /// @remarks - Computations can be costly, so a tailored implementation is more efficient than a generic one.
        ///            These utilities provide most of the tools needed to implement and optimize camera views specific to each app.
        ///          - Left-hand position: x=right / y=up / z=into
        ///          - Angles: x=yaw (left->right) / y=pitch (down->up) / z=roll (tilt clockwise)
        /*class CameraViewUtils final {
        public:
          CameraViewUtils() = delete;
          
          static inline float toRadians(float degrees) noexcept { return DirectX::XMConvertToRadians(degrees); } ///< Degrees to radians
          static inline float toDegrees(float radians) noexcept { return DirectX::XMConvertToDegrees(radians); } ///< Radians to degrees
          
          /// @brief Constrain pitch angle position to avoid reversal (maxed at vertical view) - degrees
          static inline float constrainPitchDegrees(float pitch) noexcept {
            if (pitch > 89.0f)       return 89.0f;
            else if (pitch < -89.0f) return -89.0f;
            return pitch;
          }
          /// @brief Constrain pitch angle position to avoid reversal (maxed at vertical view) - radians
          static inline float constrainPitchRadians(float pitch) noexcept {
            if (pitch > 1.55335f)       return 1.55335f;
            else if (pitch < -1.55335f) return -1.55335f;
            return pitch;
          }
          
          // -- translations --
          
          /// @brief Transform relative translation offset into absolute XYZ offset - yaw angle only (radians)
          /// @remarks Useful for first-person/third-person walking offsets (where camera pitch doesn't affect the movements). 
          ///          Can be used even if the camera has "pitch"/"roll" angles, as long as the translation is not relative to them.
          static Position toAbsoluteOffset(float frontOffset, float rightOffset, float yaw) noexcept {return Position{};}
          /// @brief Transform relative translation offset into absolute XYZ offset - yaw/pitch angles (radians)
          /// @remarks Useful for first-person/third-person walking/swimming offsets, affected by pitch angle. 
          ///          Can be used even if the camera has a "roll" angle, as long as the translation is not relative to it.
          static Position toAbsoluteOffset(float frontOffset, float rightOffset, float yaw, float pitch) noexcept {return Position{};}
          /// @brief Transform relative translation offset into absolute XYZ offset - yaw/pitch/roll angles (radians)
          /// @warning Slower computation: only used when translation is relative to the 'roll' angle too (example: plane turning left).
          static Position toAbsoluteOffset(float frontOffset, float rightOffset, const Angles& yawPitchRoll) noexcept {return Position{};}

          /// @brief Transform relative translation offset into absolute XYZ offset - yaw/pitch angles (radians)
          /// @remarks Useful for first-person/third-person offsets with a vertical component and affected by pitch angle (example: character with jet-packs). 
          ///          Can be used even if the camera has a "roll" angle, as long as the translation is not relative to it.
          /// @warning Slower computation: only used when translation has a relative vertical component (not for jumps that are always vertical).
          static Position toAbsoluteOffset(const Position& offsetFromEyeDirection, float yaw, float pitch) noexcept {return Position{};}
          /// @brief Transform relative translation offset into absolute XYZ offset - yaw/pitch/roll angles (radians)
          /// @warning Slower computation: only used when translation has a relative vertical component and is relative to the 'roll' angle (example: helicopter).
          static Position toAbsoluteOffset(const Position& offsetFromEyeDirection, const Angles& yawPitchRoll) noexcept {return Position{};}
          
          // -- rotations --
          
          /// @brief Rotate position horizontally around an axis (keep same radius from different angle)
          static inline Position rotateAround(const Position& position, float yaw, const Position& origin) noexcept {
            return Position{};
          }
          /// @brief Rotate position around an origin/focal-point (keep same radius from different angles)
          static inline Position rotateAround(const Position& position, float yaw, float pitch, const Position& origin) noexcept {
            return Position{};
          }
          
          /// @brief Create view rotation matrix from yaw angle (radians)
          static inline MatrixFloat4x4 computeRotationMatrix(float yaw) noexcept {
            return MatrixFloat4x4{};
          }
          /// @brief Create view rotation matrix from yaw/pitch angles (radians)
          static inline MatrixFloat4x4 computeRotationMatrix(float yaw, float pitch) noexcept {
            return MatrixFloat4x4{};
          }
          /// @brief Create view rotation matrix from yaw/pitch/roll angles (radians)
          static inline MatrixFloat4x4 computeRotationMatrix(float yaw, float pitch, float roll) noexcept {
            return DirectX::XMMatrixRotationRollPitchYaw(yaw, pitch, roll);
          }
          
          // -- camera view matrix --
          
          /// @brief Create camera view matrix looking at a focal-point (not transposed -> transposeMatrix required to use it in shaders)
          /// @remarks Useful for third-person views and for rotating/show-room cameras.
          static inline MatrixFloat4x4 lookAtViewMatrix(const Position& eyePosition, const Position& focalPoint, const Position& originUp) noexcept {
            return DirectX::XMMatrixLookAtLH(eyePosition, focalPoint, originUp);
          }
          /// @brief Create camera view matrix from eye position/direction (not transposed -> transposeMatrix required to use it in shaders).
          /// @remarks Useful for first-person views implemented with direction unit vectors (eyeFront/eyeRight/eyeUp, for example).
          static inline MatrixFloat4x4 lookToViewMatrix(const Position& eyePosition, const Position& frontEyeDirection, const Position& originUp) noexcept {
            return DirectX::XMMatrixLookToLH(eyePosition, frontEyeDirection, originUp);
          }
          
          /// @brief Create camera view matrix from eye position/rotation (not transposed -> transposeMatrix required to use it in shaders)
          /// @remarks Useful for first-person/cinematic views implemented with absolute rotation angles (yaw/pitch/roll, for example).
          static inline MatrixFloat4x4 rotationToViewMatrix(const Position& eyePosition, const MatrixFloat4x4& rotationMatrix) noexcept {
            return rotationMatrix * DirectX::XMMatrixTranslation(eyePosition);
          }
          /// @brief Create camera view matrix from eye position/rotation (not transposed -> transposeMatrix required to use it in shaders)
          /// @remarks Useful for first-person/cinematic views implemented with absolute rotation angles (yaw/pitch/roll, for example).
          static inline MatrixFloat4x4 rotationScalingToViewMatrix(const Position& eyePositionconst MatrixFloat4x4& rotationMatrix, float scale) noexcept {
            return DirectX::XMMatrixScaling(scale,scale,scale) * rotationMatrix * DirectX::XMMatrixTranslation(eyePosition);
          }
          /// @brief Create camera view matrix from eye position/rotation (not transposed -> transposeMatrix required to use it in shaders)
          /// @remarks Useful for first-person/cinematic views implemented with absolute rotation angles (yaw/pitch/roll, for example).
          static inline MatrixFloat4x4 rotationScalingToViewMatrix(const Position& eyePositionconst MatrixFloat4x4& rotationMatrix, const Position& scale) noexcept {
            return DirectX::XMMatrixScaling(scale.x,scale.y,scale.z) * rotationMatrix * DirectX::XMMatrixTranslation(eyePosition);
          }
          
          /// @brief Transpose camera view matrix (or model world matrix) to use it in shaders
          /// @returns Shader camera-view matrix (or model-world matrix)
          static inline MatrixFloat4x4 transposeMatrix(MatrixFloat4x4& source) noexcept { return DirectX::XMMatrixTranspose(source); }
          /// @brief Combine camera view matrix with model world matrix, then transpose result to use it in shaders
          /// @remarks - Recommended for high-poly meshes with many vertices, to speed up GPU processing of each vertex.
          ///          - Should be avoided for low-poly meshes (CPU calculations may cost more than the benefit for GPU).
          /// @returns Shader model-view matrix
          static inline MatrixFloat4x4 combineTransposeMatrices(MatrixFloat4x4& cameraView, MatrixFloat4x4& modelWorld) noexcept { 
            return DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(cameraView, modelWorld)); 
          }
        };*/
      }
    }
  }

# undef NODRAWTEXT
# undef NOGDI
# undef NOBITMAP
# undef NOMCX
# undef NOSERVICE
#endif
