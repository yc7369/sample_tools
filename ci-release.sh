#!/bin/bash
set -e
# sh release.sh push
python3 release.py

release_dir="$HOME/release/infra-release"

cd $release_dir

git tag $CI_COMMIT_TAG
git push origin $CI_COMMIT_TAG
