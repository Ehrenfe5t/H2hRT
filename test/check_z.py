import json, numpy as np
with open('output/a1_real_chain/coverage/sbr_coverage.json') as f:
    d = json.load(f)
zs = sorted(set(round(r['z'], 1) for r in d['records']))
xs = sorted(set(round(r['x'], 1) for r in d['records']))
ys = sorted(set(round(r['y'], 1) for r in d['records']))
print(f'Grid: X[{min(xs):.0f}~{max(xs):.0f}]x{len(xs)}  Y[{min(ys):.0f}~{max(ys):.0f}]x{len(ys)}  Z[{min(zs):.0f}~{max(zs):.0f}]x{len(zs)}')
print(f'Z samples: {zs[:5]} ... {zs[-5:]}')
for z in [-20,-15,-12,-10,-8,-5,-2]:
    layer = [r for r in d['records'] if abs(r['z']-z)<0.06]
    act = [r for r in layer if r['ray_hit_count']>0]
    pwr = [r['power_dBm'] for r in act] if act else []
    print(f'  Z={z:5.1f}: {len(layer):5d} Rx, {len(act):5d} active, power=[{min(pwr):.0f}~{max(pwr):.0f}]' if pwr else f'  Z={z:5.1f}: {len(layer):5d} Rx, {len(act):5d} active, NO POWER')
