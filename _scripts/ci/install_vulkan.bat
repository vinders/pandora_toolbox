@echo off

set _VULKAN_SDK_VERSION=1.2.154.1

:: fetch vulkan SDK installer
set /A retry = 1
:loop
echo "Fetching vulkan SDK installer..."
curl -L --silent --show-error --output VulkanSDK.exe "https://vulkan.lunarg.com/sdk/download/%_VULKAN_SDK_VERSION%/windows/VulkanSDK-%_VULKAN_SDK_VERSION%-Installer.exe?Human=true"

if %retry%==5 goto end 
set /a retry=%retry%+1 
if not exist ./VulkanSDK.exe goto loop
:end

if not exist ./VulkanSDK.exe (
    echo "Failed to obtain installer..."
    exit /B 1
)

:: run installer
VulkanSDK.exe /S
del /f VulkanSDK.exe
echo "Vulkan SDK install complete!"

if "%VULKAN_SDK%"=="" (
    setx VULKAN_SDK C:\VulkanSDK\%_VULKAN_SDK_VERSION%
    setx VK_SDK_PATH C:\VulkanSDK\%_VULKAN_SDK_VERSION%
    set VULKAN_SDK=C:\VulkanSDK\%_VULKAN_SDK_VERSION%
    set VK_SDK_PATH=C:\VulkanSDK\%_VULKAN_SDK_VERSION%
)
  
exit /B 0
