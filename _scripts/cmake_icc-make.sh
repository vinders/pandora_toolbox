sh ./_cmake_project_build.sh "icc-make"
if [ $? -eq 0 ]; then
    exit 0
else
    exit 1
fi
