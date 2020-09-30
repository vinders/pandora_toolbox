#!/bin/bash
if ! [ -e "./CMakeLists.txt" ]; then
  cd ..
  if ! [ -e "./CMakeLists.txt" ]; then
    cd ..
  fi
fi

if [ -z "$1" ]; then
  OUTPUT_DIR=_coverage
else
  OUTPUT_DIR=$1
fi
mkdir $OUTPUT_DIR
echo "code coverage output directory: ${OUTPUT_DIR}"

for dir in *; do
  if [ "$dir" = "*" ]; then continue; fi
  if [ -d "$dir" ] && [ -d "./${dir}/include" ] && [ -d "./${dir}/test" ]; then
    echo "code coverage: library ${dir} found."
    (cd $OUTPUT_DIR && mkdir $dir)
    sh ./_scripts/_coverage_project.sh "./${dir}/test" "_build/linux/${dir}/CMakeFiles/project.dir/test" "$(pwd)/${OUTPUT_DIR}/${dir}" "test"
    echo $(ls -lA "${OUTPUT_DIR}/${dir}" | wc -l)
  fi
done
