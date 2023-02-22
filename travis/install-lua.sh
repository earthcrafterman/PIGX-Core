#!/usr/bin/env bash

set -euxo pipefail

CXX=${CXX:-g++}
MAKE=${MAKE:-sudo make}

mkdir -p /mnt/c/vcpkg2
cd /mnt/c/vcpkg2
curl --retry 5 --connect-timeout 30 --location --remote-header-name --output installed.zip https://github.com/edo9300/edopro-vcpkg-cache/releases/download/20221218/installed_x86-windows-static.zip
unzip -uo installed.zip > /dev/null
mkdir ports
mkdir triplets
mkdir triplets/community
./vcpkg.exe integrate install