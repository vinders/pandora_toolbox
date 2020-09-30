sh ./_cmake_project_build.sh "ios"
if [ $? -eq 0 ]; then
    exit 0
else
    exit 1
fi
