#!/bin/bash

# Identify version file
FILENAME="./build_version.txt"
if ! [ -e "./CMakeLists.txt" ]; then
    cd ..
    if ! [ -e "./CMakeLists.txt" ]; then
        echo "project directory not found in current directory nor in parent directory!"
        exit 1
    fi
fi

# Get major version - count breaking changes
MAJOR=$(git rev-list -n 999999999 --grep='^major-' -i --count HEAD)
if ! [ "$MAJOR" = "0" ]; then
  MAJOR_REV=$(git rev-list -n 999999999 --grep='^major-' -i -1 HEAD)
fi

# Get minor version - count feature changes since last break
if [ -z "$MAJOR_REV" ]; then
  MINOR=$(git rev-list -n 999999999 --grep='^feat-' -i --count HEAD)
else
  MINOR=$(git rev-list -n 999999999 --grep='^feat-' -i --count ${MAJOR_REV}..HEAD)
fi
if ! [ "$MINOR" = "0" ]; then
  MINOR_REV=$(git rev-list -n 999999999 --grep='^feat-' -i -1 HEAD)
fi
if [ -z "$MINOR_REV" ]; then
  if ! [ -z "$MAJOR_REV" ]; then
    MINOR_REV=MAJOR_REV
  fi
fi

# Get patch version - count fixes since last feature
if [ -z "$MINOR_REV" ]; then
  PATCH=$(git rev-list -n 999999999 --grep='^fix-' -i --count HEAD)
else
  PATCH=$(git rev-list -n 999999999 --grep='^fix-' -i --count ${MINOR_REV}..HEAD)
fi

# Total number of commits
TOTAL=$(git rev-list -n 999999999 --count HEAD)

# Update version file
rm -f $FILENAME
echo "${MAJOR}.${MINOR}.${PATCH}.${TOTAL}" >> $FILENAME
echo "${MAJOR}.${MINOR}.${PATCH}.${TOTAL}"

exit 0
