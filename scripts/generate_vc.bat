@echo off
cd %~dp0\..
md out
md out\vc
cd out\vc
cmake -G "Visual Studio 12" -T CTP_Nov2013 ..\..
pause
