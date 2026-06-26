"""Quick verification of co/cross-pol data in RT output"""
import json, statistics

with open('output/ch3_test2/rx1/paths/precise_paths.json') as f:
    data = json.load(f)
paths = data['paths']

print("=== First 5 paths ===")
for p in paths[:5]:
    co = p.get('co_pol_power_linear', 0)
    cx = p.get('cross_pol_power_linear', 0)
    xpr = p.get('xpr_dB', 0)
    pw = p['power_linear']
    aoa_t = p.get('aoa_theta_deg', 0)
    aoa_p = p.get('aoa_phi_deg', 0)
    print(f"  path={p['path_id']:4d}: co={co:.3e}, cross={cx:.3e}, xpr={xpr:7.1f}dB, "
          f"power={pw:.3e}, AoA=({aoa_t:.0f},{aoa_p:.0f})")

co_vals = [p.get('co_pol_power_linear', 0) for p in paths]
cx_vals = [p.get('cross_pol_power_linear', 0) for p in paths]
xpr_vals = [p.get('xpr_dB', 0) for p in paths if abs(p.get('xpr_dB', 0)) < 200]
pure_co = sum(1 for p in paths if abs(p.get('xpr_dB', 0)) >= 200)

print(f"\n=== Statistics ({len(paths)} paths) ===")
print(f"Paths with co>0:    {sum(1 for c in co_vals if c > 1e-30)}")
print(f"Paths with cross>0: {sum(1 for c in cx_vals if c > 1e-30)}")
print(f"Pure co-pol (XPR>=200dB): {pure_co}")
print(f"Valid XPR values:  {len(xpr_vals)}")
if xpr_vals:
    print(f"XPR: mean={statistics.mean(xpr_vals):.1f}, median={statistics.median(xpr_vals):.1f}, "
          f"min={min(xpr_vals):.1f}, max={max(xpr_vals):.1f} dB")

# Check total co+cross vs power_linear
for p in paths[:3]:
    co = p.get('co_pol_power_linear', 0)
    cx = p.get('cross_pol_power_linear', 0)
    pw = p['power_linear']
    print(f"  co+cross={co+cx:.3e} vs power={pw:.3e}, ratio={(co+cx)/(pw+1e-30):.2f}")
