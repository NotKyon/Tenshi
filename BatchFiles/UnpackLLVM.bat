@echo off

set LLVMVER=3.6.0

echo.
echo ========================
echo Unpacking LLVM %LLVMVER%
echo ========================
echo.

set "SRCDIR=%~dp0..\ThirdParty\LLVM-%LLVMVER%"
set "BINDIR=%~dp0..\ThirdParty\Bin"

set "PAKDIR=%SRCDIR%\Download"

set "PACKAGES=lld lldb"

pushd "%SRCDIR%"
for %%A in ( %PACKAGES% llvm ) do (
	"%BINDIR%\7z.exe" x -y "-o%SRCDIR%" -- "%PAKDIR%\%%A-%LLVMVER%.src.tar.xz"
	if errorlevel 1 (
		popd
		exit /B 1
	)

	"%BINDIR%\tar.exe" --extract "--file=%%A-%LLVMVER%.src.tar"
	if errorlevel 1 (
		popd
		exit /B 1
	)

	del /Q "%SRCDIR%\%%A-%LLVMVER%.src.tar"
)
popd

set "LLVM_SRCDIR=%SRCDIR%\llvm-%LLVMVER%.src"

exit /B 0
