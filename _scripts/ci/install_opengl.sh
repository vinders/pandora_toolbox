#!/bin/bash
if ! [ "$(uname -s)" = "Darwin" ]; then
  if ! [ -f "/usr/lib/libGL.so" ]; then
    LIB_GL_PATH="$(find /usr/lib -name libGL.so* | head -n 1)"
    if ! [ "$LIB_GL_PATH" = "" ]; then
      sudo ln -s "${LIB_GL_PATH}" /usr/lib/libGL.so
  
    else
      # Red-Hat/Fedora/CentOS/...
      if [ -f /etc/redhat-release ]; then
        if ! [ test -f "/usr/lib/libGL.so" ]; then
          sudo yum install -y mesa-libGL-devel
          LIB_GL_PATH="$(find /usr/lib -name libGL.so* | head -n 1)"
        fi

      else
        # Debian/Ubuntu/...
        sudo apt-get install -y libgl1-mesa-dev
        LIB_GL_PATH="$(find /usr/lib -name libGL.so* | head -n 1)"
        if [ "$LIB_GL_PATH" = "" ]; then
          sudo apt-get install -y libgl1-mesa-glx
          LIB_GL_PATH="$(find /usr/lib -name libGL.so* | head -n 1)"
        fi
      fi
      
      # symbolic link of installed package
      if ! [ "$LIB_GL_PATH" = "" ]; then
        if ! [ "$LIB_GL_PATH" = "/usr/lib/libGL.so" ]; then
          sudo ln -s "${LIB_GL_PATH}" /usr/lib/libGL.so
        fi
      fi
    
    fi
  fi
fi
