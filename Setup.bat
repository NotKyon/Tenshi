@echo off
setlocal

set "ENV_BATCHES=DownloadWGetSources.bat App.InstallCMake.bat UnpackLLVM.bat App.InstallLLVM.bat BuildLLVM.bat"

for %%Z in ( %ENV_BATCHES% ) do (
	call "%~dp0BatchFiles\%%Z"
	if errorlevel 1 (
		echo ERROR: Batch file at "%~dp0BatchFiles\%%Z" failed.
		exit /B 1
	)
)

exit /B 0
