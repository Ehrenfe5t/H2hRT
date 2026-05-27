$url = "https://go.microsoft.com/fwlink/?linkid=2272610"
$out = "G:\RT\H2hRT-7.1-SBR-\tools\winsdk_installer.exe"

Write-Host "Downloading Windows 11 SDK installer..."
Write-Host "URL: $url"

try {
    Invoke-WebRequest -Uri $url -OutFile $out -UseBasicParsing
    if (Test-Path $out) {
        $size = (Get-Item $out).Length
        Write-Host "Downloaded: $size bytes"

        # Run installer with admin rights
        Write-Host "Launching installer (UAC prompt will appear)..."
        Start-Process -FilePath $out -ArgumentList "/quiet /norestart" -Verb RunAs -Wait

        Write-Host "Installer finished."
    }
} catch {
    Write-Host "Error: $_"
}
