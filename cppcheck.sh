#!/bin/bash

mkdir -p .cppcheck_cache &&
    cppcheck --std=c23 \
        --language=c \
        --platform=unix64 \
        --enable=all \
        --inconclusive \
        --check-level=exhaustive \
        --force \
        --inline-suppr \
        --suppress=missingIncludeSystem \
        --cppcheck-build-dir=.cppcheck_cache \
        -j $(nproc) \
        -I api \
        src api
