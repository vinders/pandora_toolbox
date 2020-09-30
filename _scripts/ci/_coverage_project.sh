#!/bin/bash
if ! [ -e "./CMakeLists.txt" ]; then
  cd ..
  if ! [ -e "./CMakeLists.txt" ]; then
    cd ..
  fi
fi

if [ -z "$1" ]; then 
  SOURCE_FILTER=`find . | egrep '\.cpp'`
else
  cd $1
  SOURCE_FILTER="*.cpp *.cxx *.c"
fi

if [ -z "$2" ]; then 
  OBJECT_DIR="."
else
  OBJECT_DIR=$2
  [ ! -d $OUTPUT_DIR ] && mkdir $OUTPUT_DIR
fi

if [ -z "$3" ]; then 
  OUTPUT_DIR="."
else
  OUTPUT_DIR=$3
  [ ! -d $OUTPUT_DIR ] && mkdir $OUTPUT_DIR
fi

if [ -z "$4" ]; then 
  PREFIX=""
else
  PREFIX="${4}_"
fi

for filename in $SOURCE_FILTER; do 
  if [ "$filename" = "*" ]; then continue; fi
  gcov -o "${OBJECT_DIR}" $filename && mv $OBJECT_DIR "${OUTPUT_DIR}/${PREFIX}"
done
