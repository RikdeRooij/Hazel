@echo off
pushd %~dp0\..\
call vendor\premake\bin\premake5.exe vs2017
popd
PAUSE
