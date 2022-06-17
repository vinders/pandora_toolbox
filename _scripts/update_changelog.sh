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

# Get last and current commit revisions (last revision read from changelog)
CURRENT_REVISION=$(git rev-parse HEAD)
if [ -e "$CHANGELOG_FILENAME" ]; then
  LAST_REVISION=$(cat "$CHANGELOG_FILENAME" | head -5 | tail -1 | awk '{ print $NF }')
  if [ -n "$LAST_REVISION" ]; then
    LAST_REVISION_TYPE=$(git cat-file -t ${LAST_REVISION})
  fi
fi

# Update and read project version
sh ./_scripts/update_version.sh
if ! [ -e "$VERSION_FILENAME" ]; then
  echo "failed to generate version file..."
  exit 1
fi
CURRENT_VERSION=$(cat "$VERSION_FILENAME" | head -1 | tail -1 | awk '{ print $NF }')

# Count all changes since last revision
if [ -n "$LAST_REVISION_TYPE" ] && [ "$LAST_REVISION_TYPE" = "commit" ]; then
  NB_BREAK_FEATURES=$(git rev-list -n 999999999 --grep='^break-feat-' -i --count ${LAST_REVISION}..HEAD)
  NB_BREAK_FIXES=$(git rev-list -n 999999999 --grep='^break-fix-' -i --count ${LAST_REVISION}..HEAD)
  NB_FEATURES=$(git rev-list -n 999999999 --grep='^feat-' -i --count ${LAST_REVISION}..HEAD)
  NB_FIXES=$(git rev-list -n 999999999 --grep='^fix-' -i --count ${LAST_REVISION}..HEAD)
else
  NB_BREAK_FEATURES=$(git rev-list -n 999999999 --grep='^break-feat-' -i --count HEAD)
  NB_BREAK_FIXES=$(git rev-list -n 999999999 --grep='^break-fix-' -i --count HEAD)
  NB_FEATURES=$(git rev-list -n 999999999 --grep='^feat-' -i --count HEAD)
  NB_FIXES=$(git rev-list -n 999999999 --grep='^fix-' -i --count HEAD)
fi

# Update changelog file
if [ $NB_BREAK_FEATURES -gt 0 ] || [ $NB_BREAK_FIXES -gt 0 ] || [ $NB_FEATURES -gt 0 ] || [ $NB_FIXES -gt 0 ]; then
  CHANGES="# Changelog\nAll notable changes will be documented in this file (generated).\n\n## ${CURRENT_VERSION}\n$(date +'%Y-%m-%d') - $CURRENT_REVISION"
  
  if [ $NB_BREAK_FEATURES -gt 0 ]; then
    CHANGES+="\n### features (major)\n"
    if [ -n "$LAST_REVISION_TYPE" ] && [ "$LAST_REVISION_TYPE" = "commit" ]; then
      BREAK_FEAT_DATA=$(git shortlog --grep='^break-feat-' -i ${LAST_REVISION}..HEAD)
    else
      BREAK_FEAT_DATA=$(git shortlog --grep='^break-feat-' -i)
    fi
    CHANGES+="${BREAK_FEAT_DATA//      break-feat-/* }"
  fi
  if [ $NB_FEATURES -gt 0 ]; then
    CHANGES+="\n### features\n"
    if [ -n "$LAST_REVISION_TYPE" ] && [ "$LAST_REVISION_TYPE" = "commit" ]; then
      FEAT_DATA=$(git shortlog --grep='^feat-' -i ${LAST_REVISION}..HEAD)
    else
      FEAT_DATA=$(git shortlog --grep='^feat-' -i)
    fi
    CHANGES+="${FEAT_DATA//      feat-/* }"
  fi
  if [ $NB_BREAK_FIXES -gt 0 ]; then
    CHANGES+="\n### fixes (major)\n"
    if [ -n "$LAST_REVISION_TYPE" ] && [ "$LAST_REVISION_TYPE" = "commit" ]; then
      BREAK_FIX_DATA=$(git shortlog --grep='^break-fix-' -i ${LAST_REVISION}..HEAD)
    else
      BREAK_FIX_DATA=$(git shortlog --grep='^break-fix-' -i)
    fi
    CHANGES+="${BREAK_FIX_DATA//      break-fix-/* }"
  fi
  if [ $NB_FIXES -gt 0 ]; then
    CHANGES+="\n### fixes\n"
    if [ -n "$LAST_REVISION_TYPE" ] && [ "$LAST_REVISION_TYPE" = "commit" ]; then
      FIX_DATA=$(git shortlog --grep='^fix-' -i ${LAST_REVISION}..HEAD)
    else
      FIX_DATA=$(git shortlog --grep='^fix-' -i)
    fi
    CHANGES+="${FIX_DATA//      fix-/* }"
  fi
  
  if [ -e "$CHANGELOG_FILENAME" ]; then
    PREV_DATA=$(tail -n +3 "${CHANGELOG_FILENAME}")
    printf "${CHANGES}\n\n---\n${PREV_DATA}" > "${CHANGELOG_FILENAME}"
  else
    printf "${CHANGES}\n\n---\n" > "${CHANGELOG_FILENAME}"
  fi
fi

exit 0
