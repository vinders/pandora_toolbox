#!/bin/bash

# Identify version file
if ! [ -e "./CMakeLists.txt" ]; then
    cd ..
    if ! [ -e "./CMakeLists.txt" ]; then
        echo "project directory not found in current directory nor in parent directory!"
        exit 1
    fi
fi

# Identify branch
BRANCH_NAME=$(git rev-parse --abbrev-ref HEAD)
if [ -n $BRANCH_NAME ] && [ "$BRANCH_NAME" = "develop" ]; then

  # Update build version + changelog
  sh ./_scripts/update_changelog.sh
  read -r CURRENT_VERSION<build_version.txt
  DIFF=$(git diff ./build_version.txt)

  # Commit changes + merge + create tag
  if [ -n DIFF ]; then
    git add build_version.txt
    git add CHANGELOG.md

    if git commit -m 'infra-ci: update changelog' && git push && git checkout master && git pull && git merge develop
    then
      CONFLICTS=$(git ls-files --unmerged)
      if [ -z "$CONFLICTS" ]; then
        git tag "v${CURRENT_VERSION}"
        git push
        git push origin "v${CURRENT_VERSION}"
        git checkout develop
      else
        echo "Merge conflicts... Operation aborted."
      fi
    else
      echo "Commit or merge failure... Operation aborted."
    fi
  else
    echo "No version change... Operation aborted."
  fi

else
  echo "Not on branch develop... Operation aborted."
fi

exit 0
