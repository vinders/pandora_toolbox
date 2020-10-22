#!/bin/bash
if [ -z "$1" ]; then
  VULKAN_VERSION=1.2.154.0
else
  VULKAN_VERSION="$1"
fi

cd $TRAVIS_BUILD_DIR/_libs

if [ "$(uname -s)" = "Darwin" ]; then
  # Mac OS X
  RETRY_COUNT=0
  until [ "$RETRY_COUNT" -ge 5 ]
  do
    wget -O vulkansdk-macos.dmg "https://vulkan.lunarg.com/sdk/download/$VULKAN_VERSION/mac/vulkansdk-macos-$VULKAN_VERSION.dmg?Human=true" \
     && sudo chmod 777 vulkansdk-macos.dmg && sudo hdiutil attach vulkansdk-macos.dmg && break

    RETRY_COUNT=$((RETRY_COUNT+1)) 
    rm $CMAKE_PACKAGE_NAME.tar.gz
    sleep 15
  done
  if [ $RETRY_COUNT = 5 ]; then
    exit 1
  fi
  
  cd /Volumes/vulkansdk-macos-$VULKAN_VERSION
  sudo python3 ./install_vulkan.py
  sh ./setup-env.sh
  cd $TRAVIS_BUILD_DIR
  sudo hdiutil detach /Volumes/vulkansdk-macos-$VULKAN_VERSION
  
else
  # Red-Hat/Fedora/CentOS/...
  if [ -f /etc/redhat-release ]; then
    sudo yum install -y vulkan vulkan-info
  else
    # Debian/Ubuntu/...
    sudo apt-get install -y libx11-dev libx11-xcb-dev libwayland-dev libxrandr-dev
    sudo apt-get install -y libvulkan1 vulkan-utils libvulkan-dev
    sudo apt-get install -y mesa-vulkan-drivers
  fi
  
  RETRY_COUNT=0
  until [ "$RETRY_COUNT" -ge 5 ]
  do
    wget -O vulkansdk-linux.run "https://vulkan.lunarg.com/sdk/download/$VULKAN_VERSION/linux/vulkansdk-linux-x86_64-$VULKAN_VERSION.run?Human=true" \
     && sudo chmod 777 vulkansdk-linux.run && break

    RETRY_COUNT=$((RETRY_COUNT+1)) 
    rm $CMAKE_PACKAGE_NAME.tar.gz
    sleep 15
  done
  if [ $RETRY_COUNT = 5 ]; then
    exit 1
  fi
  
  ./vulkansdk-linux.run
  export VULKAN_SDK=$TRAVIS_BUILD_DIR/_libs/VulkanSDK/$VULKAN_VERSION/x86_64
fi
exit 0
