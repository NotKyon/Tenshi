@echo off

echo.
echo ========================
echo Building LLVM
echo ========================
echo.

if "%CMAKE%" == "" (
	echo CMake not found or environment variable not set.
	echo Make sure to run App.InstallCMake.bat first.
	exit /B 1
)

if "%CMAKE_VS_GENERATOR%" == "" (
	call "%~dp0FindVS.bat"
	if errorlevel 1 exit /B 1
)

set LLVM_MINVSYEAR=2013
if %VS_YEAR% LSS %LLVM_MINVSYEAR% (
	echo ERROR: Found Visual Studio %VS_YEAR%, but need %LLVM_MINVSYEAR% or newer.
	exit /B 1
)

set "LLVM_SRCDIR=%~dp0..\ThirdParty\LLVM-3.6.0\llvm-3.6.0.src"
set "LLVM_DSTDIRXX=%LLVM_SRCDIR%\.Build-VS%VS_YEAR%"

set "LLVM_DSTDIR64=%LLVM_DSTDIRXX%-x64"
set "LLVM_DSTDIR32=%LLVM_DSTDIRXX%-x86"

rem Generate the 64-bit version of LLVM
if not exist "%LLVM_DSTDIR64%" (
	mkdir "%LLVM_DSTDIR64%"
	if errorlevel 1 exit /B 1
)
pushd "%LLVM_DSTDIR64%"
"%CMAKE%" -G "%CMAKE_VS_GENERATOR%" -A "x64" "%LLVM_SRCDIR%"
if errorlevel 1 (
	popd
	exit /B 1
)
popd

rem Generate the 32-bit version of LLVM
if not exist "%LLVM_DSTDIR32%" (
	mkdir "%LLVM_DSTDIR32%"
	if errorlevel 1 exit /B 1
)
pushd "%LLVM_DSTDIR32%"
"%CMAKE%" -G "%CMAKE_VS_GENERATOR%" "%LLVM_SRCDIR%"
if errorlevel 1 (
	popd
	exit /B 1
)
popd

rem Build LLVM for x86-64
pushd "%LLVM_DSTDIR64%"

setlocal
call "%VS_COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
msbuild ALL_BUILD.vcxproj
endlocal

popd

rem Build LLVM for x86-32
pushd "%LLVM_DSTDIR32%"

setlocal
call "%VS_COMNTOOLS%..\..\VC\bin\vcvars32.bat"
msbuild ALL_BUILD.vcxproj
endlocal

popd

exit /B 0
