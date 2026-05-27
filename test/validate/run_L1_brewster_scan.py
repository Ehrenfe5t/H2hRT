#!/usr/bin/env python3
"""
L1-07: Brewster Angle Scan — batch simulation across incidence angles.

Tests |Gamma_TE|^2 and |Gamma_TM|^2 behavior versus theoretical Fresnel
curves for a lossless dielectric (eps_r=5.24) wall at multiple incidence angles.

Each run varies Tx position to change incidence angle while keeping Rx fixed.
"""
import json, math, os, subprocess, sys

C0 = 299792458.0
EPS0 = 8.8541878128e-12
PI = math.pi

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(os.path.dirname(SCRIPT_DIR))
EXE = os.path.join(PROJECT_DIR, "x64", "Release", "RT.exe")
BASE_CONFIG = os.path.join(PROJECT_DIR, "test", "configs", "L1_03_dielectric_brewster.json")

def csqrt(z):
    mag = math.sqrt(abs(z))
    phase = math.atan2(z.imag, z.real) / 2
    return complex(mag*math.cos(phase), mag*math.sin(phase))

def fresnel_theory(theta_deg, eps_r, sigma, freq):
    """Compute |Gamma_TE|^2 and |Gamma_TM|^2 for given incidence angle."""
    theta = math.radians(theta_deg)
    cosI = math.cos(theta)
    sin2I = math.sin(theta)**2
    omega = 2*PI*freq
    ei = sigma/(omega*EPS0) if omega > 0 else 0
    eps_c = complex(eps_r, -ei)
    sqrt_term = csqrt(eps_c - complex(sin2I, 0))

    gamma_te = (complex(cosI,0) - sqrt_term) / (complex(cosI,0) + sqrt_term)
    gamma_tm = (complex(eps_r*cosI, -ei*cosI) - sqrt_term) / (complex(eps_r*cosI, -ei*cosI) + sqrt_term)
    return abs(gamma_te)**2, abs(gamma_tm)**2

def modify_config_tx(tx_y, run_id):
    """Create a modified config with specific Tx Y position."""
    with open(BASE_CONFIG) as f:
        cfg = json.load(f)
    cfg["app_runtime"]["run_id"] = run_id
    cfg["path_search"]["tx_y"] = tx_y
    cfg["path_search"]["rx_x"] = 1.0  # Rx on front side near wall
    cfg["path_search"]["rx_y"] = 1.0
    cfg["path_search"]["rx_z"] = 1.5
    cfg["path_search"]["enable_los"] = True
    cfg["experiment"]["experiment_tag"] = "L1_07"
    cfg["output"]["export_paths"] = True
    return cfg

def compute_incidence_angle(tx, rx, wall_x):
    """Compute incidence angle from Tx,Rx positions and wall geometry."""
    # Reflect Rx across wall at x=wall_x
    rx_mirror_x = wall_x - (rx[0] - wall_x)
    mirror_rx = (rx_mirror_x, rx[1], rx[2])

    # Ray direction from Tx toward mirror Rx
    dx = mirror_rx[0] - tx[0]; dy = mirror_rx[1] - tx[1]; dz = mirror_rx[2] - tx[2]
    length = math.sqrt(dx*dx+dy*dy+dz*dz)
    if length < 1e-9: return None
    ray_dir = (dx/length, dy/length, dz/length)

    # Incidence angle: cos(theta_i) = |dot(ray_dir, wall_normal)|
    wall_normal = (-1.0, 0.0, 0.0)
    cosI = abs(ray_dir[0] * wall_normal[0] + ray_dir[1] * wall_normal[1] + ray_dir[2] * wall_normal[2])
    return math.degrees(math.acos(min(1.0, cosI)))

def run_sim(cfg):
    """Run RT simulation with given config, return path data."""
    tmp_path = os.path.join(PROJECT_DIR, "test", "configs", f"_tmp_{cfg['app_runtime']['run_id']}.json")
    with open(tmp_path, 'w') as f:
        json.dump(cfg, f, indent=2)
    subprocess.run([EXE, tmp_path], capture_output=True, timeout=60, cwd=PROJECT_DIR)
    result_path = os.path.join(PROJECT_DIR, "output", cfg['app_runtime']['run_id'], "paths", "precise_paths.json")
    if os.path.exists(result_path):
        with open(result_path) as f:
            data = json.load(f)
        os.remove(tmp_path)
        return data
    os.remove(tmp_path)
    return None

def main():
    print("=" * 72)
    print("  L1-07: Brewster Angle Scan — |Gamma_TE|^2 vs incidence angle")
    print("=" * 72)

    eps_r, sigma, freq = 5.24, 0.0, 2.4e9
    wall_x = 2.0

    # Tx positions to achieve different incidence angles
    # Rx fixed at (1.0, 1.0, 1.5) on front side (x<2)
    # Varying Tx Y moves the incidence angle
    tx_positions = [
        (0.0, 0.1, 1.5),   # near normal -> ~0 deg
        (0.0, 0.5, 1.5),   # ~30 deg
        (0.0, 1.0, 1.5),   # ~45 deg
        (0.0, 1.5, 1.5),   # ~56 deg
        (0.0, 2.5, 1.5),   # ~68 deg (near Brewster 66.4 deg)
        (0.0, 4.0, 1.5),   # ~76 deg
    ]

    rx = (1.0, 1.0, 1.5)
    results = []

    for tx_x, tx_y, tx_z in tx_positions:
        theta_i = compute_incidence_angle((tx_x, tx_y, tx_z), rx, wall_x)
        if theta_i is None: continue

        te_theo, tm_theo = fresnel_theory(theta_i, eps_r, sigma, freq)

        run_id = f"L1_07_brewster_{int(theta_i):02d}deg"
        cfg = modify_config_tx(tx_y, run_id)
        cfg["path_search"]["tx_x"] = tx_x
        cfg["path_search"]["tx_y"] = tx_y
        cfg["path_search"]["tx_z"] = tx_z

        data = run_sim(cfg)

        if data and data.get("path_count", 0) > 0:
            paths = data.get("paths", data.get("results", []))
            refl = [p for p in paths if not p.get("is_los", False)]
            los = [p for p in paths if p.get("is_los", False)]

            result = {"theta": theta_i, "te_theo": te_theo, "tm_theo": tm_theo}

            if refl:
                p = refl[0]
                fspl_db = p.get("free_space_loss_db", 0)
                total_len = p.get("total_length_m", 0)
                power = p.get("power_linear", 0)
                if power > 0 and total_len > 0:
                    wl = C0 / freq
                    fspl_power = (wl / (4*PI*total_len))**2
                    gamma_sq_sim = power / fspl_power
                    result["gamma_sq_sim"] = gamma_sq_sim

            results.append(result)

            print(f"  theta={theta_i:.1f}deg | TE_theo={te_theo:.4f} TM_theo={tm_theo:.4f}", end="")
            if "gamma_sq_sim" in result:
                print(f" | Gamma_sim={result['gamma_sq_sim']:.4f} diff={abs(result['gamma_sq_sim']-te_theo):.4f}")
            else:
                print(f" | (no reflection path)")
        else:
            print(f"  theta={theta_i:.1f}deg | simulation failed (no paths)")

    # Summary
    print()
    print("─" * 72)
    print("  L1-07 Summary: |Gamma_TE|^2 vs incidence angle")
    print(f"  Brewster angle (theory): {math.degrees(math.atan(math.sqrt(eps_r))):.2f} deg")
    print(f"  TE at Brewster (theory): {fresnel_theory(math.degrees(math.atan(math.sqrt(eps_r))), eps_r, sigma, freq)[0]:.4f}")
    print(f"  TM at Brewster (theory): {fresnel_theory(math.degrees(math.atan(math.sqrt(eps_r))), eps_r, sigma, freq)[1]:.2e}")

    for r in results:
        sim = r.get("gamma_sq_sim", None)
        status = ""
        if sim:
            diff = abs(sim - r["te_theo"])
            status = "PASS" if diff < 0.05 else "FAIL"
        print(f"  theta={r['theta']:5.1f}deg TE_theo={r['te_theo']:.4f} sim={sim:.4f if sim else 'N/A':>8} {status}")

if __name__ == "__main__":
    main()
