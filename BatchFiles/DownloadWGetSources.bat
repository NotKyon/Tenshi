@echo off

echo.
echo ========================
echo Downloading resources
echo ========================
echo.

set BINDIR=%~dp0..\ThirdParty\Bin\
set WGETBIN=%BINDIR%wget.exe

pushd %~dp0..

set NUM_FAILED_DOWNLOADS=0
set NUM_TOTAL_DOWNLOADS=0

for /F "eol=; tokens=1,2" %%A IN ( %~dp0Input\wget-sources.txt ) DO (
	set /A NUM_TOTAL_DOWNLOADS=NUM_TOTAL_DOWNLOADS+1

	if not exist %%A (
		if not exist %%~dpA mkdir %%~dpA

		%WGETBIN% -N %%B -O %%A
		if errorlevel 1 (
			del %%A
			set /A NUM_FAILED_DOWNLOADS=NUM_FAILED_DOWNLOADS+1
		)
	)
)

if %NUM_FAILED_DOWNLOADS% NEQ 0 goto L_fail

set EXIT_CODE=0
goto L_finally

:L_fail
echo.
echo Failed to download %NUM_FAILED_DOWNLOADS%/%NUM_TOTAL_DOWNLOADS% files.
set EXIT_CODE=1

:L_finally
popd
exit /B %EXIT_CODE%
