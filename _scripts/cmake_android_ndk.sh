if [ -z "$ANDROID_NDK_ROOT" ]; then
    if [ -z "$ANDROID_NDK_HOME" ]; then
        echo "Missing environment variable: ANDROID_NDK_ROOT"
        exit 1
    fi
fi
if [ -z "$1" ]; then
    echo "Missing argument 1 to identify Android API level (value example: 26)"
    exit 1
fi
if [ -z "$2" ]; then
    echo "No argument 2 to identify ABI type: will default to 'arm64-v8a'"
    ABI_TYPE=arm64-v8a
    USE_NEON=TRUE
else
    ABI_TYPE=$2
    if [ "$2" = "arm" ]; then
        USE_NEON=FALSE
    elif [ "$2" = "armeabi" ]; then
        USE_NEON=FALSE
    elif [ "$2" = "armeabi-v7a" ]; then
        USE_NEON=FALSE
    elif [ "$2" = "armeabi-v6" ]; then
        USE_NEON=FALSE
    elif [ "$2" = "x86_64" ]; then
        USE_NEON=FALSE
    elif [ "$2" = "x86" ]; then
        USE_NEON=FALSE
    elif [ "$2" = "mips64" ]; then
        USE_NEON=FALSE
    elif [ "$2" = "mips" ]; then
        USE_NEON=FALSE
    else
        USE_NEON=TRUE
    fi
fi

if ! [ -e "./_cmake_project_build.sh" ]; then
    cd ./_scripts
fi
sh ./_cmake_project_build.sh "android" $1 $ABI_TYPE $USE_NEON
if [ $? -eq 0 ]; then
    exit 0
else
    exit 1
fi
