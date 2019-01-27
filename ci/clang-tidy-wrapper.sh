#!/bin/sh

set -e

clang-tidy-6.0 -warnings-as-errors='*' "$@"
