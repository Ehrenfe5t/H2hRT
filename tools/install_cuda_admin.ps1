$installer = "G:\RT\H2hRT-7.1-SBR-\tools\cuda_12.6.0_windows_network.exe"

Write-Host "Launching CUDA 12.6 installer with elevation..."
Write-Host "Installer: $installer"
Write-Host ""

# Run with elevation and wait
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $installer
$psi.Arguments = "-s"
$psi.UseShellExecute = $true
$psi.Verb = "runas"
$psi.WindowStyle = "Normal"

$process = [System.Diagnostics.Process]::Start($psi)
Write-Host "Process started (PID: $($process.Id)), waiting..."
$process.WaitForExit()

Write-Host ""
Write-Host "CUDA installer exited with code: $($process.ExitCode)"

if ($process.ExitCode -eq 0) {
    Write-Host "Installation successful!"
    $cudaPath = "${env:ProgramFiles}\NVIDIA GPU Computing Toolkit\CUDA\v12.6"
    if (Test-Path $cudaPath) {
        Write-Host "CUDA installed at: $cudaPath"
        $nvcc = Join-Path $cudaPath "bin\nvcc.exe"
        if (Test-Path $nvcc) {
            & $nvcc --version
        }
    }
}
