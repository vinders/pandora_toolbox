sh ./_cmake_project_build.sh "xcode"
if [ $? -eq 0 ]; then
    exit 0
else
    exit 1
fi
