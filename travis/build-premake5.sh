#!/usr/bin/env bash

set -euxo pipefail

PREMAKE_FLAGS=""

./premake5.exe vs2019 --oldwindows=true;
dotnet msbuild -p:Configuration=release -p:Platform=Win32 -t:ocgcoreshared  -verbosity:minimal -p:EchoOff=true ./build/ocgcore.sln;
