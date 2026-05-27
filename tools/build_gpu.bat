@echo off
setlocal enabledelayedexpansion
REM ==============================================
REM  H2hRT GPU Accelerated Build
REM  Platform: RTX 3060 12GB (sm_86)
REM ==============================================

set "VS_VCVARS=F:\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat"
set "MSBUILD=F:\vs2022\ide\MSBuild\Current\Bin\MSBuild.exe"
set "NVCC=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6\bin\nvcc.exe"
set "OPTIX_INC=G:\RT\H2hRT-7.1-SBR-\tools\include"
set "PROJECT_DIR=G:\RT\H2hRT-7.1-SBR-"
set "OUT_DIR=%PROJECT_DIR%\x64\Release"
set "PTX_SRC=%PROJECT_DIR%\core\query\optix_device.cu"
set "PTX_DST=%OUT_DIR%\optix_device.ptx"
set "SLN=%PROJECT_DIR%\RT.sln"

REM Ensure CUDA_PATH is set (may not propagate to cmd subprocesses)
set "CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6"
set "CUDA_PATH_V12_6=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6"

REM Windows SDK is at non-standard path: F:\Windows Kits\10\
set "WINSDK_ROOT=F:\Windows Kits\10"
set "WINSDK_VER=10.0.22621.0"
set "WINSDK_INC=%WINSDK_ROOT%\Include\%WINSDK_VER%"
set "WINSDK_UCRT=%WINSDK_INC%\ucrt"
set "WINSDK_SHARED=%WINSDK_INC%\shared"
set "WINSDK_UM=%WINSDK_INC%\um"

echo ==============================================
echo   H2hRT - GPU Accelerated Build
echo   Platform: RTX 3060 12GB (sm_86)
echo ==============================================
echo.

REM Step 0: Setup VS environment
if not defined VSCMD_VER (
    echo [0/3] Setting up VS2022 environment...
    call "%VS_VCVARS%" >nul 2>&1
    if errorlevel 1 (
        echo ERROR: Cannot setup VS2022 environment.
        pause
        exit /b 1
    )
    echo [0/3] VS2022 environment ready.
    echo   Windows SDK: %WINSDK_ROOT%
) else (
    echo [0/3] VS2022 environment already active.
)

REM Ensure Windows SDK and MSVC paths are set
set "MSVC_INC=F:\vs2022\ide\VC\Tools\MSVC\14.40.33807\include"
set "MSVC_ATLMFC=F:\vs2022\ide\VC\Tools\MSVC\14.40.33807\atlmfc\include"
set "MSVC_BIN=F:\vs2022\ide\VC\Tools\MSVC\14.40.33807\bin\Hostx64\x64"
set "WINSDK_BIN=%WINSDK_ROOT%\bin\%WINSDK_VER%\x64"
set "INCLUDE=%MSVC_INC%;%MSVC_ATLMFC%;%WINSDK_UCRT%;%WINSDK_SHARED%;%WINSDK_UM%;%INCLUDE%"
set "PATH=%MSVC_BIN%;%WINSDK_BIN%;%PATH%"

REM Step 1: Compile OptiX PTX
echo.
echo [1/3] Compiling OptiX device PTX...
echo   - sbr_gpu_device.cu
"%NVCC%" -arch=sm_86 -ptx -DNDEBUG -O3 -use_fast_math --expt-relaxed-constexpr -I"%OPTIX_INC%" -o "%OUT_DIR%\sbr_gpu_device.ptx" "%PROJECT_DIR%\core\query\sbr_gpu_device.cu"
if errorlevel 1 (echo ERROR: sbr_gpu_device.ptx failed)
echo   - optix_device.cu
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

echo   nvcc: %NVCC%
echo   output: %PTX_DST%

"%NVCC%" ^
    -arch=sm_86 ^
    -ptx ^
    -DNDEBUG ^
    -O3 ^
    -use_fast_math ^
    --expt-relaxed-constexpr ^
    -I"%OPTIX_INC%" ^
    -o "%PTX_DST%" ^
    "%PTX_SRC%"

if errorlevel 1 (
    echo.
    echo ERROR: PTX compilation failed.
    pause
    exit /b 1
)
echo [1/3] PTX compiled successfully.

REM Step 2: Build C++/CUDA project
echo.
echo [2/3] Building project (MSBuild)...

"%MSBUILD%" "%SLN%" ^
    /p:Configuration=Release /p:Platform=x64 /t:Build /m /v:minimal ^
    /p:WindowsSdkDir="%WINSDK_ROOT%" ^
    /p:WindowsSDKVersion=%WINSDK_VER%

if errorlevel 1 (
    echo.
    echo BUILD FAILED.
    pause
    exit /b 1
)
echo [2/3] Build succeeded.

REM Step 3: Quick verification
echo.
echo [3/3] Running smoke test ^(L1-01^)...
cd /d "%PROJECT_DIR%"
"%OUT_DIR%RT.exe" test\configs\L1_01_free_space.json 2>&1 | findstr /C:"search_paths" /C:"A1RealChainSummary" /C:"FATAL"

echo.
echo ==============================================
echo   BUILD COMPLETE
echo ==============================================
echo   RT.exe: %OUT_DIR%RT.exe
echo   PTX:    %PTX_DST%
echo ==============================================
echo.
echo To run full L1 regression:
echo   cd /d %PROJECT_DIR%
echo   x64\Release\RT.exe test\configs\L1_01_free_space.json
echo   ^(repeat for L1_02 through L1_06^)
echo ==============================================
endlocal
exit /b 0
