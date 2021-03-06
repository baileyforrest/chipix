#!/bin/bash

BLACKLIST_DIRS=(
  'third_party'
  'testdata'
)

SCRIPT_DIR=$(dirname "$0")
cd $SCRIPT_DIR/..

cmd="find "
for dir in ${BLACKLIST_DIRS[@]}; do
  cmd="$cmd -not \( -path ./$dir -prune \) "
done

cmd="$cmd -type f -name '*.h' -o -name '*.c' -o -name '*.cc'"
eval $cmd
