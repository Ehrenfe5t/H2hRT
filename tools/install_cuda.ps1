$installer = "G:\RT\H2hRT-7.1-SBR-\tools\cuda_12.6.0_windows_network.exe"

Write-Host "Starting CUDA 12.6 silent install..."
Write-Host "This will download ~3GB of components and install them."
Write-Host "Components: nvcc, cudart, visual_studio_integration"
Write-Host ""

# Run silent install (no driver, we already have 566.36)
$process = Start-Process -FilePath $installer -ArgumentList "-s" -Wait -PassThru -NoNewWindow

if ($process.ExitCode -eq 0) {
    Write-Host "CUDA 12.6 installation completed successfully!"

    # Verify installation
    $cudaPath = "${env:ProgramFiles}\NVIDIA GPU Computing Toolkit\CUDA\v12.6"
    if (Test-Path $cudaPath) {
        Write-Host "CUDA installed at: $cudaPath"
        $nvcc = Join-Path $cudaPath "bin\nvcc.exe"
        if (Test-Path $nvcc) {
            Write-Host "nvcc found: $nvcc"
            & $nvcc --version
        }
    } else {
        Write-Host "Checking alternative CUDA locations..."
        Get-ChildItem "${env:ProgramFiles}\NVIDIA GPU Computing Toolkit\CUDA\" -Directory -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "Found: $($_.FullName)" }
    }
} else {
    Write-Host "CUDA installation exited with code: $($process.ExitCode)"
}

Write-Host ""
Write-Host "Installation script complete."
