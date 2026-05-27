$downloadDir = "G:\RT\H2hRT-7.1-SBR-\tools"
New-Item -ItemType Directory -Force -Path $downloadDir -ErrorAction SilentlyContinue | Out-Null
$installer = Join-Path $downloadDir "cuda_12.6.0_windows_network.exe"

if (Test-Path $installer) {
    $file = Get-Item $installer
    Write-Host "CUDA installer already exists: $($file.Length) bytes"
} else {
    Write-Host "Downloading CUDA 12.6 network installer (~31MB)..."
    Invoke-WebRequest -Uri "https://developer.download.nvidia.cn/compute/cuda/12.6.0/network_installers/cuda_12.6.0_windows_network.exe" -OutFile $installer -UseBasicParsing
    if (Test-Path $installer) {
        $file = Get-Item $installer
        Write-Host "Download complete: $($file.Length) bytes"
    } else {
        Write-Host "Download failed!"
        exit 1
    }
}

Write-Host "Installer path: $installer"
