if ! [ -e "./_cmake_project_build.sh" ]; then
    cd ./_scripts
fi
sh ./_cmake_project_build.sh "unix-make-cpp14"
if [ $? -eq 0 ]; then
    exit 0
else
    exit 1
fi
