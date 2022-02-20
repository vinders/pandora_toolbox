/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - resource file IO (shaders, image files)
*******************************************************************************/
#ifdef _P_SHADER_PRECOMPILED
# include <cstdlib>
# include <memory>
#endif
#include <stdexcept>
#include <io/file_system_io.h>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <DDSTextureLoader.h>
# include <WICTextureLoader.h>
#else
# include <gli/gli.hpp>
#endif
#include "file_io.h"

using namespace video_api;

// -- file path discovery -- ---------------------------------------------------

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  std::wstring getResourceDirectory() {
    static const wchar_t* dir = (!pandora::io::verifyFileSystemAccessMode("logo_big.png", pandora::io::FileSystemAccessMode::read)
                               && pandora::io::verifyFileSystemAccessMode("../logo_big.png", pandora::io::FileSystemAccessMode::read))
                              ? L"../" : L"";
    return std::wstring(dir);
  }
#else
  std::string getResourceDirectory() {
    static const char* dir = (!pandora::io::verifyFileSystemAccessMode("logo_big.png", pandora::io::FileSystemAccessMode::read)
                            && pandora::io::verifyFileSystemAccessMode("../logo_big.png", pandora::io::FileSystemAccessMode::read))
                           ? "../" : "";
    return std::string(dir);
  }
#endif


// -- file IO -- ---------------------------------------------------------------

#ifdef _P_SHADER_PRECOMPILED
  void readBinaryShaderFile(const _PATH_CHAR* filePath, std::unique_ptr<uint8_t[]>& outBuffer, size_t& outLength) {
#   if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
      FILE* descriptor = nullptr;
      _wfopen_s(&descriptor, filePath, L"rb");
#   else
      FILE* descriptor = fopen(filePath, "rb");
#   endif
    if (descriptor == nullptr)
      throw std::runtime_error("Shader: failed to read binary file");

    fseek(descriptor, 0, SEEK_END);
    outLength = (size_t)ftell(descriptor);
    fseek(descriptor, 0, SEEK_SET);
    if (outLength == 0)
      throw std::runtime_error("Shader: empty file");

    try {
      outBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[outLength]());
      if (fread(outBuffer.get(), outLength, 1, descriptor) != (size_t)1u)
        throw std::runtime_error("Shader: invalid file");
      fclose(descriptor);
    }
    catch (...) {
      fclose(descriptor);
      throw;
    }
  }
#endif

// ---

// --> texture & sprite file loaders differ between APIs
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  void readTextureFile(const std::wstring& commonFilePath, const wchar_t* fileSuffix, TextureMapId id,
                       DeviceHandle device, ID3D11Resource** outTextureRes, ID3D11ShaderResourceView** outResourceView) {
    std::wstring texturePath = commonFilePath + fileSuffix;

    auto alphaMode = DirectX::DDS_ALPHA_MODE::DDS_ALPHA_MODE_STRAIGHT;
    if (FAILED(DirectX::CreateDDSTextureFromFileEx(device, texturePath.c_str(), 0, D3D11_USAGE_IMMUTABLE,
                                                   D3D11_BIND_SHADER_RESOURCE, 0, 0, true,
                                                   outTextureRes, outResourceView, &alphaMode)))
      throw std::runtime_error(std::string("Could not load texture ") + toString(id));
  }
  void readSpriteFile(const wchar_t* imagePath, SpriteId id, DeviceHandle device, 
                      ID3D11Resource** outImageRes, ID3D11ShaderResourceView** outResourceView) {
    if (FAILED(DirectX::CreateWICTextureFromFileEx(device, imagePath, 0, D3D11_USAGE_IMMUTABLE,
                                                   D3D11_BIND_SHADER_RESOURCE, 0, 0,
                                                   DirectX::WIC_LOADER_FLAGS::WIC_LOADER_FORCE_RGBA32,
                                                   outImageRes, outResourceView)))
      throw std::runtime_error(std::string("Could not load sprite image ") + toString(id));
  }
  
#else
  void readTextureFile(const std::string& commonFilePath, const char_t* fileSuffix, TextureMapId id,
                       DeviceHandle device, TextureHandle* outTextureRes, TextureView* outResourceView) {
    //vulkan: not yet implemented
    //...
  }
  void readSpriteFile(const char_t* imagePath, SpriteId id, DeviceHandle device, 
                      TextureHandle* outImageRes, TextureView* outResourceView) {
    //vulkan: not yet implemented
    //...
  }
#endif
