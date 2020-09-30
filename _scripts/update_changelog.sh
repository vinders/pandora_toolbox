#!/bin/bash

# Identify version file
FILENAME="./CHANGELOG.md"
if ! [ -e "./CMakeLists.txt" ]; then
    cd ..
    if ! [ -e "./CMakeLists.txt" ]; then
        echo "project directory not found in current directory nor in parent directory!"
        exit 1
    fi
fi

# update/get project version + revision
sh ./_scripts/update_version.sh
read -r CURRENT_VERSION<build_version.txt
CURRENT_REVISION=$(git rev-parse HEAD)
# get last logged revision
LAST_REVISION=$(cat $FILENAME | head -5 | tail -1 | awk '{ print $NF }')
if [ -n "$LAST_REVISION" ]; then
  LAST_REVISION_TYPE=$(git cat-file -t ${LAST_REVISION})
fi

# count changes
if [ -n "$LAST_REVISION_TYPE" ] && [ "$LAST_REVISION_TYPE" = "commit" ]; then
  NB_MAJOR=$(git rev-list -n 999999999 --grep='^major-' -i --count ${LAST_REVISION}..HEAD)
  NB_FEATURES=$(git rev-list -n 999999999 --grep='^feat-' -i --count ${LAST_REVISION}..HEAD)
  NB_FIXES=$(git rev-list -n 999999999 --grep='^fix-' -i --count ${LAST_REVISION}..HEAD)
else
  NB_MAJOR=$(git rev-list -n 999999999 --grep='^major-' -i --count HEAD)
  NB_FEATURES=$(git rev-list -n 999999999 --grep='^feat-' -i --count HEAD)
  NB_FIXES=$(git rev-list -n 999999999 --grep='^fix-' -i --count HEAD)
fi

# Update changelog file
if [ $NB_MAJOR -gt 0 ] || [ $NB_FEATURES -gt 0 ] || [ $NB_FIXES -gt 0 ]; then
  CHANGES="# Changelog\nAll notable changes will be documented in this file (generated).\n\n## ${CURRENT_VERSION}\n$(date +'%Y-%m-%d') - $CURRENT_REVISION"
  if [ -n "$LAST_REVISION_TYPE" ] && [ "$LAST_REVISION_TYPE" = "commit" ]; then
    if [ $NB_MAJOR -gt 0 ]; then
      CHANGES+="\n### major changes\n"
      MAJOR_DATA=$(git shortlog --grep='^major-' -i ${LAST_REVISION}..HEAD)
      CHANGES+="${MAJOR_DATA//      major-/* }"
    fi
    if [ $NB_FEATURES -gt 0 ]; then
      CHANGES+="\n### features\n"
      FEAT_DATA=$(git shortlog --grep='^feat-' -i ${LAST_REVISION}..HEAD)
      CHANGES+="${FEAT_DATA//      feat-/* }"
    fi
    if [ $NB_FIXES -gt 0 ]; then
      CHANGES+="\n### fixes\n"
      FIX_DATA=$(git shortlog --grep='^fix-' -i ${LAST_REVISION}..HEAD)
      CHANGES+="${FIX_DATA//      fix-/* }"
    fi
  else
    if [ $NB_MAJOR -gt 0 ]; then
      CHANGES+="\n### major changes\n"
      MAJOR_DATA=$(git shortlog --grep='^major-' -i)
      CHANGES+="${MAJOR_DATA//      major-/* }"
    fi
    if [ $NB_FEATURES -gt 0 ]; then
      CHANGES+="\n### features\n"
      FEAT_DATA=$(git shortlog --grep='^feat-' -i)
      CHANGES+="${FEAT_DATA//      feat-/* }"
    fi
    if [ $NB_FIXES -gt 0 ]; then
      CHANGES+="\n### fixes\n"
      FIX_DATA=$(git shortlog --grep='^fix-' -i)
      CHANGES+="${FIX_DATA//      fix-/* }"
    fi
  fi

  printf "${CHANGES}\n\n---\n$(tail -n +3 ${FILENAME})" > $FILENAME
fi

exit 0
