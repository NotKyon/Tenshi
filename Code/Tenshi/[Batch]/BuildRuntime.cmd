@echo off

set MINGW_BIN=C:\MinGW\bin
set MSYS_BIN=C:\msys\bin

set ORIG_PATH=%PATH%
set PATH=%MINGW_BIN%;%MSYS_BIN%;%PATH%

set RT_SRCDIR=%~dp0..\Runtime

pushd %RT_SRCDIR%
%MSYS_BIN%\sh.exe "build.sh"
popd

set PATH=%ORIG_PATH%
