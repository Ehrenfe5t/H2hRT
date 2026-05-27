@echo off
echo ===============================================
echo  H2hRT - Full Build with GPU Acceleration
echo ===============================================

call "F:\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo ERROR: vcvars64.bat failed
    exit /b 1
)

echo.
echo MSVC environment loaded successfully.
echo.

echo Step 1/2: Building project...
"F:\vs2022\ide\MSBuild\Current\Bin\MSBuild.exe" "G:\RT\H2hRT-7.1-SBR-\RT.sln" /p:Configuration=Release /p:Platform=x64 /t:Build
if errorlevel 1 (
    echo.
    echo BUILD FAILED
    exit /b 1
)

echo.
echo ===============================================
echo  BUILD SUCCEEDED
echo ===============================================
echo.
echo Step 2/2: Running L1 regression...
echo.
cd /d "G:\RT\H2hRT-7.1-SBR-"
x64\Release\RT.exe test\configs\L1_01_free_space.json 2>&1 | findstr /C:"search_paths" /C:"A1RealChainSummary" /C:"FATAL"
echo.
echo Done.
