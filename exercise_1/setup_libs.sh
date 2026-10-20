#!/usr/bin/env bash
# Copies the course libraries (arrays.h, linearalgebra.h, mt19937ar.h, ...)
# from the IntroductionToMPI submodule into this exercise's include/ folder.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
LIBS="$ROOT/IntroductionToMPI/Examples/Libraries"

# Submodule not downloaded yet (repo cloned without --recurse-submodules)
if [ ! -d "$LIBS" ]; then
    echo "Submodule missing, initializing it..."
    git -C "$ROOT" submodule update --init IntroductionToMPI
fi

mkdir -p "$HERE/include"
cp -v "$LIBS"/* "$HERE/include/"
echo "Done: libraries copied to $HERE/include"