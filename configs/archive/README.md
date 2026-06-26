# v11 Config Archive

Archived configs — NOT compatible with v11 P2P main chain.
Kept for historical reference only.

## Archived from configs/app/

| File | Reason |
|------|--------|
| `meeting_412_v9_full.json` | v9 legacy full config (SearchEngine + Coverage) |
| `v10_coverage_template.json` | v10 coverage template (SBR coverage-only mode) |
| `v10_sbr_ptp_template.json` | v10 SBR P2P template (superseded by v11_ptp_*.json) |
| `v10_finechannel_template.json` | v10 FineChannel template (superseded) |

## To archive manually

Copy the following from `configs/app/` into the appropriate archive subdirectory:

```powershell
# Archive old v9/v10 configs
Copy-Item configs\app\meeting_412_v9_full.json configs\archive\old_v9_v10\
Copy-Item configs\app\v10_*.json configs\archive\old_v9_v10\

# Archive old coverage configs
Copy-Item configs\meeting_coverage_hires.json configs\archive\old_coverage\
```

## Keeping (active v11 baseline + comparison)

- `configs/app/v11_ptp_412.json` — v11 P2P 412 scene
- `configs/app/v11_ptp_meeting.json` — v11 P2P meeting scene  
- `configs/app/sbr_compare_412.json` — baseline comparison (412)
- `configs/app/sbr_compare_meeting.json` — baseline comparison (meeting)
- `configs/app/sbr_compare_412_rx01_diff_smoke.json` — diffraction smoke test
- `configs/app/meeting_412.json` — generic meeting/412 config
