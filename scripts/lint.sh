#!/bin/bash

set -e

SCRIPT_DIR=$(dirname "$0")
cd $SCRIPT_DIR/..

DIRS="core arch"
FILES=$(find $DIRS -name "*.h" -o -name "*.cc")

./third_party/styleguide/cpplint/cpplint.py --verbose=0 \
    --filter=-legal/copyright,-build/header_guard,-build/c++11,-build/include_order,-whitespace/newline,-runtime/references \
    --root=$PWD $FILES

./third_party/styleguide/cpplint/cpplint.py --verbose=0 \
    --filter=-legal/copyright,-build/header_guard,-build/c++11,-build/include_order,-whitespace/newline,-runtime/references,-build/include_what_you_use \
    --root=$PWD $(find libcxx -name "*.h" -o -name "*.cc")
