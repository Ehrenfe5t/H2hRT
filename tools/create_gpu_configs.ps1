# Copy L1 configs and add GPU acceleration
$configs = @(
    "L1_01_free_space.json",
    "L1_02_pec_reflection.json",
    "L1_03_dielectric_brewster.json",
    "L1_04_transmission.json",
    "L1_05_pec_wedge.json",
    "L1_06_transmission.json"
)

$srcDir = "G:\RT\H2hRT-7.1-SBR-\test\configs"
$gpuAccel = '"acceleration": { "backend": "GPU_OptiX", "gpu_device_id": 0, "gpu_batch_size": 1000000, "gpu_use_rt_core": true }'

foreach ($cfg in $configs) {
    $src = Join-Path $srcDir $cfg
    $dst = Join-Path $srcDir ($cfg -replace '\.json$', '_gpu.json')
    if (Test-Path $src) {
        $content = Get-Content $src -Raw
        # Change run_id
        $content = $content -replace '"run_id": "[^"]*"', '"run_id": "' + ($cfg -replace '\.json$', '_gpu') + '"'
        # Add acceleration before the closing }
        if ($content -match '\n\}') {
            $content = $content -replace '\n\}', ",`n  $gpuAccel`n}"
        }
        Set-Content -Path $dst -Value $content -Encoding UTF8
        Write-Host "Created: $dst"
    }
}
