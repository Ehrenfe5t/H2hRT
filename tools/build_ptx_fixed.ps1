$projectRoot = "G:\RT\H2hRT-7.1-SBR-"
$cudaPath = "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6"
$optxInc = "$projectRoot\tools\include"
$msvcPath = "F:\vs2022\ide\VC\Tools\MSVC\14.40.33807\bin\Hostx64\x64"

$nvcc = "$cudaPath\bin\nvcc.exe"
$source = "$projectRoot\core\query\optix_device.cu"
$outDir = "$projectRoot\x64\Release"
$output = "$outDir\optix_device.ptx"

Write-Host "Compiling OptiX PTX for RTX 3060 (sm_86)..."
Write-Host "nvcc:     $nvcc"
Write-Host "compiler: $msvcPath"
Write-Host ""

# Set up PATH to include VS host compiler
$env:PATH = "$msvcPath;" + $env:PATH

$args = @(
    "-arch=sm_86",
    "-ptx",
    "-DNDEBUG",
    "-O3",
    "-use_fast_math",
    "--expt-relaxed-constexpr",
    "-I", "`"$optxInc`"",
    "-o", "`"$output`"",
    "`"$source`""
)

$cmd = "& `"$nvcc`" $($args -join ' ')"
Write-Host $cmd

$result = cmd.exe /c $cmd 2>&1
Write-Host $result

if ($LASTEXITCODE -eq 0 -and (Test-Path $output)) {
    $sz = (Get-Item $output).Length
    Write-Host "`nPTX build SUCCESS: $output ($sz bytes)"
} else {
    Write-Host "`nPTX build FAILED"
}
