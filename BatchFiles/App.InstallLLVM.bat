@echo off

if "%LLVMVER%" == "" set LLVMVER=3.6.0

echo.
echo ========================
echo Installing LLVM %LLVMVER%
echo ========================
echo.

if "%LLVM_BINDIR%" == "" (
	set "LLVM_BINDIR=%ProgramFiles(x86)%\LLVM\bin"
) else (
	if not exist "%LLVM_BINDIR%" (
		echo LLVM %LLVMVER% not found at "%LLVM_BINDIR%"
		echo Please change or remove your LLVM_BINDIR environment variable.
		exit /B 1
	)
)

if "%CLANG%" == "" set "CLANG=%LLVM_BINDIR%\clang.exe"
if "%CLANG_CL%" == "" set "CLANG_CL=%LLVM_BINDIR%\clang-cl.exe"
if "%CLANG_FORMAT%" == "" set "CLANG_FORMAT=%LLVM_BINDIR%\clang-format.exe"
if "%LLD%" == "" set "LLD=%LLVM_BINDIR%\lld.exe"

if exist "%CLANG%" exit /B 0

set "LLVM_INSTALLER=%~dp0..\ThirdParty\LLVM-3.6.0\Download\LLVM-%LLVMVER%-win32.exe"

if not exist "%LLVM_INSTALLER%" (
	echo LLVM %LLVMVER% installer not found at "%LLVM_INSTALLER%"...
	echo Please run DownloadWGetSources.bat
	exit /B 1
)

start /wait "%LLVM_INSTALLER%" /S

if errorlevel 1 (
	echo Failed to install LLVM %LLVMVER%.
	exit /B 1
)

exit /B 0
