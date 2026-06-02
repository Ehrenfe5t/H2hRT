# v9 Validation Summary — all suite

**Timestamp**: 2026-06-03T04:05:51.274794
**Backend**: cpu
**Results**: 33/34 passed, 0 failed, 1 skipped (1 critical)
**Paper-ready**: NO
**Real RT.exe cases**: 11  |  **Analytic cases**: 23

## Results
| ID | Source | Result | Detail |
|----|--------|--------|--------|
| S-1 | ANA | PASS | |Γ|=0.333333 |
| S-2 | ANA | PASS | theta_t=19.47 |
| S-3 | ANA | PASS | FSPL=41.99dB |
| A-1 | ANA | PASS | |G|=0.9998, phase=180.0 |
| A-2 | ANA | PASS | |G_TE|=0.5120, |G_TM|=0.2622 |
| A-3 | ANA | PASS | |G_TE|=0.5464, |G_TM|=0.2985 |
| A-4 | ANA | PASS | residual=5.55e-17 |
| A-5 | ANA | PASS | sin2=1.1250, TIR=True |
| A-6 | ANA | PASS | FSPL=40.05dB |
| A-7 | ANA | PASS | |G_TM|=1.24e-16 at Brewster |
| G-1 | RT | PASS | OK: paths=1 |
| G-2 | RT | PASS | OK: paths=2 |
| G-3 | RT | PASS | OK: paths=1 tx=1 |
| G-4 | RT | PASS | OK: paths=1 |
| G-5-prod | RT | PASS | OK: paths=2544 diff=2469 |
| G-6 | RT | SKIP* | config not found: output/G6/rx1/paths/precise_paths.json |
| G-TK | RT | PASS | K1=1 <= K8=1 <= KX(exhaustive)=1 |
| EM-1 | ANA | PASS | E+(-E)=0 |
| EM-2 | ANA | PASS | R+T=1.0000 |
| EM-3 | ANA | PASS | ph=180.0 |
| EM-4 | ANA | PASS | V-Tx*H-Rx=0 |
| EM-D1 | ANA | PASS | vr=1.000 |
| EM-D2 | ANA | PASS | V-inc*H-rx=0 |
| EM-D3 | ANA | PASS | same=1.00>opp=0.00 |
| EM-D3 | ANA | PASS | same=1.00>opp=0.00 |
| W-1 | ANA | PASS | |H(1GHz)|=1.500000 |
| W-2 | ANA | PASS | coherent=0, incoherent=0.5 |
| W-3 | ANA | PASS | 200MHz -> 5ns |
| W-4 | RT | PASS | files=6/6 meta=OK |
| B-1 | RT | PASS | RT.exe rc=1 |
| B-2 | ANA | PASS | CUDA toolkit found |
| B-3 | ANA | PASS | 10um tolerance |
| B-4 | RT | PASS | paths=2==2 |
| SBR-1 | RT | PASS | pwr=51.74uW paths=2 |

## Skipped Cases
- **G-6** [BLOCKS thesis]: config not found: output/G6/rx1/paths/precise_paths.json

## Failed Cases
None.

## Paper Experiment Readiness
**PAPER_READY=false**. Reasons:
  - G-6: config not found: output/G6/rx1/paths/precise_paths.json