@echo off
call "F:\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set "INCLUDE=%INCLUDE%;F:\Windows Kits\10\Include\10.0.22621.0\ucrt;F:\Windows Kits\10\Include\10.0.22621.0\shared;F:\Windows Kits\10\Include\10.0.22621.0\um"

echo === CUDA PATH ===
echo CUDA_PATH=%CUDA_PATH%
echo.

echo === Building with diagnostics ===
"F:\vs2022\ide\MSBuild\Current\Bin\MSBuild.exe" "G:\RT\H2hRT-7.1-SBR-\RT.vcxproj" /p:Configuration=Release /p:Platform=x64 /t:Build /v:normal /p:WindowsSdkDir="F:\Windows Kits\10" /p:WindowsSDKVersion=10.0.22621.0 2>&1
