@echo off

rem :TryVS15
rem if "%VS150COMNTOOLS%" == "" goto TryVS2015
rem set VS_YEAR=2017
rem set VS_VER=15
rem set "VS_COMNTOOLS=%VS150COMNTOOLS%"
rem set "CMAKE_VS_GENERATOR=Visual Studio 15"
rem goto Done

:TryVS2015
if "%VS140COMNTOOLS%" == "" goto TryVS2013
set VS_YEAR=2015
set VS_VER=14
set "VS_COMNTOOLS=%VS140COMNTOOLS%"
goto Done

:TryVS2013
if "%VS120COMNTOOLS%" == "" goto TryVS2012
set VS_YEAR=2013
set VS_VER=12
set "VS_COMNTOOLS=%VS120COMNTOOLS%"
goto Done

:TryVS2012
if "%VS110COMNTOOLS%" == "" goto TryVS2010
set VS_YEAR=2012
set VS_VER=11
set "VS_COMNTOOLS=%VS110COMNTOOLS%"
goto Done

:TryVS2010
if "%VS100COMNTOOLS%" == "" goto NoVSFound
set VS_YEAR=2010
set VS_VER=10
set "VS_COMNTOOLS=%VS100COMNTOOLS%"
goto Done

:NoVSFound
echo ERROR: Visual Studio (2010, 2012, 2013, or 2015) not found!
exit /B 1

:Done
set AXLIBS_VSVER=%VS_YEAR%
if "%CMAKE_VS_GENERATOR%" == "" set "CMAKE_VS_GENERATOR=Visual Studio %VS_VER% %VS_YEAR%"

exit /B 0
