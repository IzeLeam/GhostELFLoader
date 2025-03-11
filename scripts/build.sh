#!/bin/bash

set -xe

mkdir -p build_pipeline
mkdir -p artifacts

MAKEFILE_LINK="./Makefile.pipeline"
[[ -f $MAKEFILE_LINK ]] || ln -s $INTERNAL_MAKEFILE $MAKEFILE_LINK
make -f $MAKEFILE_LINK BUILD="-" pipeline_build