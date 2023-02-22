#!/usr/bin/env bash

set -euxo pipefail

curl --retry 5 --connect-timeout 30 --location --remote-header-name --remote-name https://github.com/premake/premake-core/releases/download/v5.0.0-alpha14/premake-5.0.0-alpha14-windows.zip
unzip -uo premake-5.0.0-alpha14-windows.zip