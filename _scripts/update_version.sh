#!/bin/bash

VERSION_FILENAME="./Version.cmake"
CHANGELOG_FILENAME="./CHANGELOG.md"

# Identify project directory
if ! [ -e "./CMakeLists.txt" ]; then
  cd ..
  if ! [ -e "./CMakeLists.txt" ]; then
    echo "project directory not found in current directory nor in parent directory!"
    exit 1
  fi
fi

# Get last commit revision + last project version (read from changelog)
if [ -e "$CHANGELOG_FILENAME" ]; then
  LAST_VERSION=$(cat "$CHANGELOG_FILENAME" | head -4 | tail -1 | awk '{ print $NF }')
  if [ -n "$LAST_VERSION" ]; then
    IFS='.'
    read -ra LAST_VERSION_ARRAY <<< "$LAST_VERSION"
    LAST_VERSION_MAJOR="${LAST_VERSION_ARRAY[0]}"
    LAST_VERSION_MINOR="${LAST_VERSION_ARRAY[1]}"
    LAST_VERSION_PATCH="${LAST_VERSION_ARRAY[2]}"
    LAST_VERSION_TOTAL="${LAST_VERSION_ARRAY[3]}"
    
    LAST_REVISION=$(cat "$CHANGELOG_FILENAME" | head -5 | tail -1 | awk '{ print $NF }')
    if [ -z "$LAST_REVISION" ] || [ -z "$LAST_VERSION_TOTAL" ]; then
      unset LAST_VERSION_MAJOR
      unset LAST_VERSION_MINOR
      unset LAST_VERSION_PATCH
      unset LAST_VERSION_TOTAL
      unset LAST_REVISION
    else
      echo "> old version: ${LAST_VERSION_MAJOR}.${LAST_VERSION_MINOR}.${LAST_VERSION_PATCH}.${LAST_VERSION_TOTAL}"
    fi
  fi
fi


# -- READ VERSION BASED ON COMMIT PREFIXES -- ----------------------------------

# Find major commits - count breaking changes ("break-...")
if [ -z "$LAST_REVISION" ]; then
  MAJOR_NB=$(git rev-list -n 999999999 --grep='^break-' -i --count HEAD)
else
  MAJOR_NB=$(git rev-list -n 999999999 --grep='^break-' -i --count ${LAST_REVISION}..HEAD)
fi
if ! [ "$MAJOR_NB" = "0" ]; then
  MAJOR_REV=$(git rev-list -n 999999999 --grep='^break-' -i -1 HEAD)
fi
# Compute major version
if [ -z "$LAST_VERSION_MAJOR" ]; then
  MAJOR_VERSION=$(expr $MAJOR_NB)
else
  MAJOR_VERSION=$(expr $LAST_VERSION_MAJOR + $MAJOR_NB)
fi

# Find minor commits - count features since MAJOR_REV ("feat-...")
if [ -z "$MAJOR_REV" ]; then
  if [ -z "$LAST_REVISION" ]; then
    MINOR_NB=$(git rev-list -n 999999999 --grep='^feat-' -i --count HEAD)
  else
    MINOR_NB=$(git rev-list -n 999999999 --grep='^feat-' -i --count ${LAST_REVISION}..HEAD)
  fi
else
  MINOR_NB=$(git rev-list -n 999999999 --grep='^feat-' -i --count ${MAJOR_REV}..HEAD)
fi
if ! [ "$MINOR_NB" = "0" ]; then
  MINOR_REV=$(git rev-list -n 999999999 --grep='^feat-' -i -1 HEAD)
fi
if [ -z "$MINOR_REV" ]; then
  if ! [ -z "$MAJOR_REV" ]; then
    MINOR_REV=MAJOR_REV
  fi
fi
# Compute minor version
if [ -z "$LAST_VERSION_MINOR" ] || ! [ "$MAJOR_NB" = "0" ]; then
  MINOR_VERSION=$MINOR_NB
else
  MINOR_VERSION=$(expr $LAST_VERSION_MINOR + $MINOR_NB)
fi

# Find patch commits - count fixes since last feature ("patch-...")
if [ -z "$MINOR_REV" ]; then
  if [ -z "$LAST_REVISION" ]; then
    PATCH_NB=$(git rev-list -n 999999999 --grep='^fix-' -i --count HEAD)
  else
    PATCH_NB=$(git rev-list -n 999999999 --grep='^fix-' -i --count ${LAST_REVISION}..HEAD)
  fi
else
  PATCH_NB=$(git rev-list -n 999999999 --grep='^fix-' -i --count ${MINOR_REV}..HEAD)
fi
# Compute patch version
if [ -z "$LAST_VERSION_PATCH" ] || ! [ "$MAJOR_NB" = "0" ] || ! [ "$MINOR_NB" = "0" ]; then
  PATCH_VERSION=$PATCH_NB
else
  PATCH_VERSION=$(expr $LAST_VERSION_PATCH + $PATCH_NB)
fi

# Find total number of commits
if [ -z "$LAST_REVISION" ]; then
  TOTAL_NB=$(git rev-list -n 999999999 --count HEAD)
else
  TOTAL_NB=$(git rev-list -n 999999999 --count ${LAST_REVISION}..HEAD)
fi
# Compute total commits
if [ -z "$LAST_VERSION_TOTAL" ]; then
  TOTAL_COMMITS=$TOTAL_NB
else
  TOTAL_COMMITS=$(expr $LAST_VERSION_TOTAL + $TOTAL_NB)
fi


# -- GENERATE VERSION FILE -- --------------------------------------------------

if [ -e "$VERSION_FILENAME" ]; then
  rm -f "${VERSION_FILENAME}"
fi
VERSION_STRING="${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}.${TOTAL_COMMITS}"
printf "# ${VERSION_STRING}\nset(CWORK_BUILD_VERSION ${VERSION_STRING})\n" > "${VERSION_FILENAME}"
echo "> new version: ${VERSION_STRING}"

exit 0
