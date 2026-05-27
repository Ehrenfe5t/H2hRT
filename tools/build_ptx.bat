@echo off
REM v8: Build OptiX device PTX — requires VS2022 + CUDA 12.6 + OptiX 7.7

set VS_VCVARS=F:\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6
set OPTIX_INC=G:\RT\H2hRT-7.1-SBR-\tools\include
set PROJECT_ROOT=G:\RT\H2hRT-7.1-SBR-

echo ===============================================
echo  Building OptiX Device PTX
echo ===============================================
echo.

REM Setup VS2022 environment
call "%VS_VCVARS%" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Cannot setup VS2022 environment
    exit /b 1
)

echo MSVC environment loaded.
echo.

REM Compile PTX
set SRC=%PROJECT_ROOT%core\query\optix_device.cu
set DST=%PROJECT_ROOT%x64\Release\optix_device.ptx

echo Source:  %SRC%
echo Output:  %DST%
echo.

"%CUDA_PATH%\bin\nvcc.exe" ^
    -arch=sm_86 ^
    -ptx ^
    -DNDEBUG ^
    -O3 ^
    -use_fast_math ^
    --expt-relaxed-constexpr ^
    -I"%OPTIX_INC%" ^
    -o "%DST%" ^
    "%SRC%"

if errorlevel 1 (
    echo.
    echo PTX build FAILED (exit code: %errorlevel%)
    exit /b %errorlevel%
)

echo.
echo PTX build SUCCESS: %DST%
if exist "%DST%" (
    for %%A in ("%DST%") do echo Size: %%~zA bytes
)
