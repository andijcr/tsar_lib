#!/usr/bin/env bash

set -e

CONTAINER=andrea/clang

echo "Container to be used: $CONTAINER."
#docker pull $CONTAINER
echo

SCRIPT=$(readlink -f $0)
SCRIPT_PATH=`dirname $SCRIPT`

echo "building container for compilation"
docker build -t $CONTAINER $SCRIPT_PATH

SOURCE_PATH=`dirname $SCRIPT_PATH`

echo "Triggering the build..."
docker run -v $SOURCE_PATH:/source $CONTAINER sh -c "/source/phoenix/build.sh"
echo "Completed docker run."
echo
