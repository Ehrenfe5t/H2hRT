$installer = "G:\RT\H2hRT-7.1-SBR-\tools\cuda_12.6.0_windows_network.exe"

Write-Host "============================================"
Write-Host " CUDA 12.6 Installation Script"
Write-Host "============================================"
Write-Host ""
Write-Host "Installer: $installer"
Write-Host "This will trigger a UAC prompt."
Write-Host "Please click 'Yes' when the User Account Control dialog appears."
Write-Host ""

# Use Start-Process with RunAs verb for elevation
try {
    $process = Start-Process -FilePath $installer -ArgumentList "-s" -Verb RunAs -Wait -PassThru
    Write-Host ""
    Write-Host "CUDA installer finished with exit code: $($process.ExitCode)"

    if ($process.ExitCode -eq 0) {
        Write-Host "Installation appears successful!"

        # Refresh environment variables
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

        # Check CUDA paths
        $cudaPaths = @(
            "${env:ProgramFiles}\NVIDIA GPU Computing Toolkit\CUDA\v12.6",
            "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6"
        )
        foreach ($cudaPath in $cudaPaths) {
            if (Test-Path $cudaPath) {
                Write-Host ""
                Write-Host "CUDA installed at: $cudaPath"
                $nvcc = Join-Path $cudaPath "bin\nvcc.exe"
                if (Test-Path $nvcc) {
                    Write-Host "nvcc: $nvcc"
                    & $nvcc --version 2>&1 | ForEach-Object { Write-Host $_ }
                }
                break
            }
        }
    } elseif ($process.ExitCode -eq $null) {
        Write-Host "Process did not return exit code - UAC may have been declined."
    } else {
        Write-Host "Installation may have failed. Check CUDA installer logs."
    }
} catch {
    Write-Host "Error: $_"
    Write-Host "You can manually install by running: $installer"
    Write-Host "Right-click -> Run as Administrator"
}
