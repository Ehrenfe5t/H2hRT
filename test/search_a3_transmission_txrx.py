#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import json
import pathlib
import re
import subprocess
from itertools import product


ROOT = pathlib.Path(r"F:\RT\RT")
CONFIG_PATH = ROOT / "configs" / "app" / "minimal.json"
EXE_PATH = ROOT / "x64" / "Debug" / "RT.exe"


TX_CANDIDATES_X = [1.0, 1.5, 2.0, 2.5, 3.0, 4.0]
TX_CANDIDATES_Y = [0.8, 1.0, 1.2, 1.5, 2.0]
TX_CANDIDATES_Z = [-6.0, -7.0, -8.0, -9.0, -10.0]

RX_CANDIDATES_X = [12.0, 13.0, 14.0, 15.0, 16.0, 17.0]
RX_CANDIDATES_Y = [0.8, 1.0, 1.2, 1.5, 2.0]
RX_CANDIDATES_Z = [-10.0, -11.0, -12.0, -13.0, -14.0]


def load_config() -> dict:
    return json.loads(CONFIG_PATH.read_text(encoding="utf-8"))


def save_config(config: dict) -> None:
    CONFIG_PATH.write_text(json.dumps(config, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def run_case(tx, rx):
    cfg = load_config()
    cfg["path_search"]["debug_tx_x"] = tx[0]
    cfg["path_search"]["debug_tx_y"] = tx[1]
    cfg["path_search"]["debug_tx_z"] = tx[2]
    cfg["path_search"]["debug_rx_x"] = rx[0]
    cfg["path_search"]["debug_rx_y"] = rx[1]
    cfg["path_search"]["debug_rx_z"] = rx[2]
    save_config(cfg)

    proc = subprocess.run(
        [str(EXE_PATH), "configs/app/minimal.json"],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="ignore",
        timeout=60,
    )
    output = (proc.stdout or "") + "\n" + (proc.stderr or "")

    transmission_selfcheck_ok = "TransmissionSelfCheck: next_states=1, failure_reasons=0" in output or "TransmissionSelfCheck: next_states=" in output
    a1_ok = "A1 real production chain closed loop completed." in output

    paths = 0
    mixed = 0
    m = re.search(r"paths=(\d+)", output)
    if m:
        paths = int(m.group(1))
    m = re.search(r"mixed_path_generated=(\d+)", output)
    if m:
        mixed = int(m.group(1))

    return {
        "tx": tx,
        "rx": rx,
        "paths": paths,
        "mixed": mixed,
        "transmission_selfcheck_ok": transmission_selfcheck_ok,
        "a1_ok": a1_ok,
        "returncode": proc.returncode,
        "output": output,
    }


def main():
    best = None
    best_score = (-1, -1)

    for tx in product(TX_CANDIDATES_X, TX_CANDIDATES_Y, TX_CANDIDATES_Z):
        for rx in product(RX_CANDIDATES_X, RX_CANDIDATES_Y, RX_CANDIDATES_Z):
            result = run_case(tx, rx)
            score = (1 if result["a1_ok"] else 0, result["paths"])
            slim = {
                "tx": result["tx"],
                "rx": result["rx"],
                "paths": result["paths"],
                "mixed": result["mixed"],
                "a1_ok": result["a1_ok"],
                "returncode": result["returncode"],
            }
            print(slim)
            if score > best_score:
                best = result
                best_score = score
            if result["a1_ok"] and result["paths"] > 0:
                print("FOUND", slim)
                return 0

    if best is not None:
        print("BEST", {
            "tx": best["tx"],
            "rx": best["rx"],
            "paths": best["paths"],
            "mixed": best["mixed"],
            "a1_ok": best["a1_ok"],
            "returncode": best["returncode"],
        })
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
