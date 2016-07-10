@echo off

set CMAKE_VER=3.2.2

echo.
echo ========================
echo Installing CMake %CMAKE_VER%
echo ========================
echo.

if "%CMAKE%" == "" (
	set "CMAKE=%ProgramFiles(x86)%\CMake\bin\cmake.exe"
) else (
	if not exist "%CMAKE%" (
		echo CMake %CMAKE_VER% not found at "%CMAKE%"...
		echo Please change or remove your CMAKE environment variable
		exit /B 1
	)
)

if exist "%CMAKE%" (
	exit /B 0
)

set "CMAKE_INSTALLER=%~dp0..\ThirdParty\Installers\cmake-%CMAKE_VER%-win32-x86.exe"

if not exist "%CMAKE_INSTALLER%" (
	echo CMake %CMAKE_VER% installer not found at "%CMAKE_INSTALLER%"...
	echo Please run DownloadWGetSources.bat
	exit /B 1
)

start /wait "%CMAKE_INSTALLER%" /S

if errorlevel 1 (
	echo Failed to install CMake %CMAKE_VER%.
	exit /B 1
)

if not exist "%CMAKE%" (
	echo CMake %CMAKE_VER% not found even after installing...
	echo Please set the CMAKE environment variable to the CMake executable.
	exit /B 1
)

exit /B 0
