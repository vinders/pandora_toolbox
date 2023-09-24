/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - resource file IO (shaders, image files)
*******************************************************************************/
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

// --> sprite file loaders differ between APIs
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  Texture2D readImageFile(const wchar_t* imagePath, ImageId id, DeviceHandle device) {
    ID3D11Resource* imageRes = nullptr;
    ID3D11ShaderResourceView* resourceView = nullptr;
    if (FAILED(DirectX::CreateWICTextureFromFileEx((ID3D11Device*)device, imagePath, 0, D3D11_USAGE_IMMUTABLE,
                                                   D3D11_BIND_SHADER_RESOURCE, 0, 0,
                                                   DirectX::WIC_LOADER_FLAGS::WIC_LOADER_FORCE_SRGB,
                                                   &imageRes, &resourceView))) {
      throw std::runtime_error(std::string("Could not load image ") + toString(id));
    }
    
    D3D11_TEXTURE2D_DESC descriptor{};
    ((ID3D11Texture2D*)imageRes)->GetDesc(&descriptor);
    return Texture2D((TextureHandle)imageRes, (TextureView)resourceView, descriptor.Width*4, descriptor.Height, 1);
  }
  
#else
  Texture2D readImageFile(const char_t* imagePath, ImageId id, DeviceHandle device) {
    //vulkan: not yet implemented
    //...
  }
#endif
