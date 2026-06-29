param(
    [string]$Experiments = "E1,E2,E3,C1,C2,C3,X1",
    [string]$RtExe = ".\x64\Release\RT.exe",
    [switch]$Resume,
    [switch]$CleanOutputs
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path "$scriptDir\..\.."
$manifestPath = Join-Path $scriptDir "experiment_manifest.json"
$logPath = Join-Path $scriptDir "experiment_log.csv"
$runManifestPath = Join-Path $scriptDir "experiment_run_manifest.json"
$manifest = Get-Content $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
$selected = @($Experiments.Split(",") | ForEach-Object { $_.Trim().ToUpperInvariant() })
$cases = @($manifest.cases | Where-Object { $selected -contains $_.group.ToUpperInvariant() })

if ($cases.Count -eq 0) {
    throw "No experiment cases selected."
}

$exePath = Resolve-Path (Join-Path $projectRoot $RtExe)
$exeHash = (Get-FileHash $exePath -Algorithm SHA256).Hash
$gitCommit = (& git -C $projectRoot rev-parse HEAD 2>$null)
$gitDirty = [bool](& git -C $projectRoot status --porcelain 2>$null)
$sessionId = (Get-Date).ToUniversalTime().ToString("yyyyMMddTHHmmssZ")

$existing = @()
if ($Resume -and (Test-Path $logPath)) {
    $existing = @(Import-Csv $logPath)
}
$rows = [System.Collections.Generic.List[object]]::new()
foreach ($row in $existing) { $rows.Add($row) }

$runMeta = [ordered]@{
    schema_version = "v11.5"
    session_id = $sessionId
    started_utc = (Get-Date).ToUniversalTime().ToString("o")
    selected_groups = $selected
    executable = $exePath.Path
    executable_sha256 = $exeHash
    git_commit = $gitCommit
    git_dirty = $gitDirty
    case_count = $cases.Count
    completed_case_count = 0
    failed_case_count = 0
}
$runMeta | ConvertTo-Json -Depth 5 | Set-Content $runManifestPath -Encoding UTF8

$caseIndex = 0
foreach ($case in $cases) {
    $caseIndex++
    $configPath = Join-Path $projectRoot $case.config
    $configHash = (Get-FileHash $configPath -Algorithm SHA256).Hash
    $alreadyDone = @($existing | Where-Object {
        $_.run_id -eq $case.run_id -and $_.config_sha256 -eq $configHash -and $_.exit_code -eq "0"
    }).Count -gt 0
    if ($Resume -and $alreadyDone) {
        Write-Host "[$caseIndex/$($cases.Count)] $($case.run_id) SKIP (matching successful run)" -ForegroundColor DarkGray
        continue
    }

    $outputDir = Join-Path $projectRoot "output\$($case.run_id)"
    if ($CleanOutputs -and (Test-Path $outputDir)) {
        Remove-Item -LiteralPath $outputDir -Recurse -Force
    }

    Write-Host "[$caseIndex/$($cases.Count)] $($case.run_id)" -ForegroundColor Cyan
    $started = (Get-Date).ToUniversalTime()
    $timer = [Diagnostics.Stopwatch]::StartNew()
    & $exePath $configPath
    $exitCode = $LASTEXITCODE
    $timer.Stop()

    $channelDir = Join-Path $outputDir "Tx01-Rx01\channel"
    $statsPath = Join-Path $channelDir "channel_stats.json"
    $xprPath = Join-Path $channelDir "xpr_stats.json"
    $megPath = Join-Path $channelDir "meg.json"
    $stats = if (Test-Path $statsPath) { Get-Content $statsPath -Raw | ConvertFrom-Json } else { $null }
    $xpr = if (Test-Path $xprPath) { Get-Content $xprPath -Raw | ConvertFrom-Json } else { $null }
    $meg = if (Test-Path $megPath) { Get-Content $megPath -Raw | ConvertFrom-Json } else { $null }

    $rows.Add([pscustomobject][ordered]@{
        session_id = $sessionId
        experiment = $case.group
        case_id = $case.case_id
        run_id = $case.run_id
        config = $case.config
        config_sha256 = $configHash
        executable_sha256 = $exeHash
        started_utc = $started.ToString("o")
        exit_code = $exitCode
        duration_s = [math]::Round($timer.Elapsed.TotalSeconds, 3)
        path_count = if ($stats) { $stats.valid_path_count } else { $null }
        total_power_linear = if ($stats) { $stats.total_power_linear } else { $null }
        rms_delay_spread_s = if ($stats) { $stats.rms_delay_spread_s } else { $null }
        k_factor_dB = if ($stats) { $stats.k_factor_dB } else { $null }
        xpr_mean_dB = if ($xpr) { $xpr.mean_dB } else { $null }
        meg_dB = if ($meg) { $meg.meg_dB } else { $null }
    })
    $rows | Export-Csv $logPath -NoTypeInformation -Encoding UTF8

    if ($exitCode -eq 0 -and $stats) {
        $runMeta.completed_case_count++
        Write-Host "PASS paths=$($stats.valid_path_count), duration=$([math]::Round($timer.Elapsed.TotalSeconds,1))s" -ForegroundColor Green
    } else {
        $runMeta.failed_case_count++
        Write-Host "FAIL exit=$exitCode" -ForegroundColor Red
    }
    $runMeta | ConvertTo-Json -Depth 5 | Set-Content $runManifestPath -Encoding UTF8
}

$runMeta.finished_utc = (Get-Date).ToUniversalTime().ToString("o")
$runMeta | ConvertTo-Json -Depth 5 | Set-Content $runManifestPath -Encoding UTF8
Write-Host "Completed $($runMeta.completed_case_count)/$($cases.Count), failed $($runMeta.failed_case_count)." -ForegroundColor Cyan
if ($runMeta.failed_case_count -gt 0) { exit 2 }

