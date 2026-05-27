# v8 Phase 3: Build OptiX device PTX from optix_device.cu
# Compiles PTX for RTX 3060 (sm_86) using CUDA 12.6 nvcc

param(
    [string]$Config = "Release",
    [string]$CudaPath = "",
    [string]$OptiXInclude = ""
)

$projectRoot = Split-Path -Parent $PSScriptRoot

# Detect CUDA path
if (-not $CudaPath) {
    $defaultCuda = "${env:ProgramFiles}\NVIDIA GPU Computing Toolkit\CUDA\v12.6"
    if (Test-Path $defaultCuda) {
        $CudaPath = $defaultCuda
    } else {
        Write-Error "CUDA 12.6 not found. Specify -CudaPath or install CUDA 12.6."
        exit 1
    }
}

# Detect OptiX include path
if (-not $OptiXInclude) {
    $optixPaths = @(
        "${projectRoot}\tools\include",
        "${env:ProgramData}\NVIDIA Corporation\OptiX SDK 7.7\include",
        "C:\ProgramData\NVIDIA Corporation\OptiX SDK 7.7\include"
    )
    foreach ($p in $optixPaths) {
        if (Test-Path (Join-Path $p "optix.h")) {
            $OptiXInclude = $p
            break
        }
    }
    if (-not $OptiXInclude) {
        Write-Error "OptiX SDK 7.7 include not found. Specify -OptiXInclude."
        exit 1
    }
}

$nvcc = Join-Path $CudaPath "bin\nvcc.exe"
if (-not (Test-Path $nvcc)) {
    Write-Error "nvcc not found at: $nvcc"
    exit 1
}

$source = Join-Path $projectRoot "core\query\optix_device.cu"
$outDir = Join-Path $projectRoot "x64\$Config"
if (-not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Force -Path $outDir | Out-Null
}
$output = Join-Path $outDir "optix_device.ptx"

Write-Host "============================================"
Write-Host " Building OptiX Device PTX"
Write-Host "============================================"
Write-Host "CUDA:       $CudaPath"
Write-Host "OptiX:      $OptiXInclude"
Write-Host "Source:     $source"
Write-Host "Output:     $output"
Write-Host ""

if ($Config -eq "Debug") {
    $args = @(
        "-arch=sm_86",
        "-ptx",
        "-D_DEBUG",
        "-G",
        "-I`"$OptiXInclude`"",
        "-o", "`"$output`"",
        "`"$source`""
    )
} else {
    $args = @(
        "-arch=sm_86",
        "-ptx",
        "-DNDEBUG",
        "-O3",
        "-use_fast_math",
        "-I`"$OptiXInclude`"",
        "-o", "`"$output`"",
        "`"$source`""
    )
}

$cmd = "& `"$nvcc`" $($args -join ' ')"
Write-Host "Command: $cmd"
Write-Host ""

Invoke-Expression $cmd

if ($LASTEXITCODE -eq 0) {
    $ptxSize = (Get-Item $output).Length
    Write-Host ""
    Write-Host "PTX build SUCCESS: $output ($ptxSize bytes)"
} else {
    Write-Host ""
    Write-Error "PTX build FAILED (exit code: $LASTEXITCODE)"
    exit 1
}
