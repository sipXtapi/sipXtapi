#! /bin/bash

# Script to check that "make clean" really does remove all build artifacts.

# Temporary directory under which the source trees will be placed.
T="${TMPDIR:-/tmp}/$$clean-check"

# The directory containing this script.
D="$( cd "$( dirname "$0" )" ; pwd )"
# The name of Makefile.devel in the same directory as this script.
MAKEFILE="$D/Makefile.devel"

# Show the user what we are doing, as some of these commands take a long time.
set -x
# Stop if there is an error.
set -e

# Make the temporary directory.
mkdir "$T"
cd "$T"

# Make a clean checkout of the source.
mkdir "$T/clean"
make -f "$MAKEFILE" -C "$T/clean" checkout

# Copy the checkout to the build directory.
cp -a "$T/clean" "$T/build"
# Build everything.
make -f "$MAKEFILE" -C "$T/clean" build-only
# Clean the build directory..
make -f "$MAKEFILE" -C "$T/clean" clean

# Get the list of files from both directories and take the difference.
( cd "$T/clean" ; find . -type f ) >"$T/clean-files"
( cd "$T/build" ; find . -type f ) >"$T/build-files"
comm -13 "$T/clean-files" "$T/build-files" >"$T/uncleaned-files"
comm -23 "$T/clean-files" "$T/build-files" >"$T/destroyed-files"

# Print the message.
echo "----- The following files are build artifacts not removed by 'make clean':"
cat "$T/uncleaned-files"
echo "----- [end]"

echo "----- The following files are source-controlled but are removed by 'make clean':"
cat "$T/destroyed-files"
echo "----- [end]"

# Delete the temporary directory.
#rm -rf "$T"
