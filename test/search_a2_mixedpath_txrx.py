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


TX_CANDIDATES_X = [2.5, 3.5, 4.0, 5.0, 6.0]
TX_CANDIDATES_Y = [1.0, 1.5, 2.0]
TX_CANDIDATES_Z = [-6.0, -7.0, -8.0, -9.0]

RX_CANDIDATES_X = [14.0, 15.0, 16.0, 16.5, 17.5]
RX_CANDIDATES_Y = [1.0, 1.5, 2.0]
RX_CANDIDATES_Z = [-11.0, -12.0, -13.0, -14.0]


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

    mixed = 0
    paths = 0
    accepted = 0
    states = 0

    m = re.search(r"mixed_path_generated=(\d+)", output)
    if m:
        mixed = int(m.group(1))
    m = re.search(r"paths=(\d+)", output)
    if m:
        paths = int(m.group(1))
    m = re.search(r"accepted_states=(\d+)", output)
    if m:
        accepted = int(m.group(1))
    m = re.search(r"states=(\d+)", output)
    if m:
        states = int(m.group(1))

    ok = "A1 real production chain closed loop completed." in output
    return {
        "tx": tx,
        "rx": rx,
        "mixed_path_generated": mixed,
        "paths": paths,
        "accepted_states": accepted,
        "states": states,
        "a1_ok": ok,
        "returncode": proc.returncode,
    }


def main():
    best = None
    best_score = (-1, -1, -1)

    for tx in product(TX_CANDIDATES_X, TX_CANDIDATES_Y, TX_CANDIDATES_Z):
        for rx in product(RX_CANDIDATES_X, RX_CANDIDATES_Y, RX_CANDIDATES_Z):
            result = run_case(tx, rx)
            score = (
                result["mixed_path_generated"],
                result["accepted_states"],
                result["paths"],
            )
            print(result)
            if result["a1_ok"] and score > best_score:
                best = result
                best_score = score
            if result["a1_ok"] and result["mixed_path_generated"] > 0:
                print("FOUND", result)
                return 0

    print("BEST", best)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
