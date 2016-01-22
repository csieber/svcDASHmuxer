#!/usr/bin/env bash

trap 'exit' ERR

cd /svcDASHmuxer

premake4 gmake

cd build/

make

echo "Provision (user) completed."
