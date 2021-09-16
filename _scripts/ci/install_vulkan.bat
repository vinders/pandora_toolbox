@echo off

set _VULKAN_SDK_VERSION=1.2.189.0

:: fetch vulkan SDK installer
echo "Fetching vulkan SDK installer..."
if not exist ./VulkanSDK-1.2.exe (
    curl -L --silent --show-error --output VulkanSDK-1.2.exe "https://vulkan.lunarg.com/sdk/download/%_VULKAN_SDK_VERSION%/windows/VulkanSDK-%_VULKAN_SDK_VERSION%-Installer.exe?Human=true"
    if not exist ./VulkanSDK-1.2.exe (
        echo "Failed to obtain installer..."
        exit /B 1
    )
)

:: run installer
echo "Starting install..."
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" /v ConsentPromptBehaviorAdmin /t REG_DWORD /d 00000000 /f
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" /v EnableLUA /t REG_DWORD /d 00000000 /f
start /wait "VulkanSDK-1.2.exe /S"
if not exist C:\VulkanSDK\%_VULKAN_SDK_VERSION%\ (
    echo "Failed to install..."
    exit /B 1
)

echo "Vulkan SDK install complete!"
if "%VULKAN_SDK%"=="" (
    setx VULKAN_SDK C:\VulkanSDK\%_VULKAN_SDK_VERSION%
    setx VK_SDK_PATH C:\VulkanSDK\%_VULKAN_SDK_VERSION%
    set VULKAN_SDK=C:\VulkanSDK\%_VULKAN_SDK_VERSION%
    set VK_SDK_PATH=C:\VulkanSDK\%_VULKAN_SDK_VERSION%
)
  
exit /B 0
