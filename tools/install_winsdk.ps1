# Install Windows 10/11 SDK via VS Installer (requires admin)
# This will trigger a UAC prompt — click Yes to proceed

$vsInstaller = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"
$vsPath = "F:\vs2022\ide"

# Try multiple SDK component IDs (Windows 11 SDK 22621, then Windows 10 SDK 20348, then 19041)
$sdkComponents = @(
    "Microsoft.VisualStudio.Component.Windows11SDK.22621",
    "Microsoft.VisualStudio.Component.Windows10SDK.20348",
    "Microsoft.VisualStudio.Component.Windows10SDK.19041"
)

Write-Host "============================================"
Write-Host " Windows SDK Installation"
Write-Host "============================================"
Write-Host "VS Installer: $vsInstaller"
Write-Host "VS Path:      $vsPath"
Write-Host ""
Write-Host "Trying to install Windows SDK..."
Write-Host "This will trigger a UAC prompt."
Write-Host "Please click 'Yes' when the User Account Control dialog appears."
Write-Host ""

$installed = $false

foreach ($sdkComp in $sdkComponents) {
    Write-Host "Attempting: $sdkComp"

    try {
        $process = Start-Process -FilePath $vsInstaller `
            -ArgumentList "modify --installPath `"$vsPath`" --add `"$sdkComp`" --quiet --norestart" `
            -Verb RunAs -Wait -PassThru

        Write-Host "Exit code: $($process.ExitCode)"

        if ($process.ExitCode -eq 0) {
            Write-Host "SDK component installed: $sdkComp"
            $installed = $true
            break
        }
    } catch {
        Write-Host "Error: $_"
    }
}

if ($installed) {
    Write-Host ""
    Write-Host "SDK installation completed. Please restart your terminal/Git Bash."
    Write-Host "Then run: G:\RT\H2hRT-7.1-SBR-\tools\build_gpu.bat"
} else {
    Write-Host ""
    Write-Host "SDK installation may have failed or the UAC prompt was declined."
    Write-Host ""
    Write-Host "Manual installation:"
    Write-Host "1. Open Start Menu -> 'Visual Studio Installer'"
    Write-Host "2. Click 'Modify' on Visual Studio 2022"
    Write-Host "3. Go to 'Individual components' tab"
    Write-Host "4. Search 'Windows 11 SDK' or 'Windows 10 SDK'"
    Write-Host "5. Check and install"
}
