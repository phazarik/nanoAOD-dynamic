#!/usr/bin/env python3

#------------------------------------------------------------------------------
# Run the NanoAOD analysis for one or all configured samples.
#
# 1) Run a specific sample:
#   python3 runAll.py --sample DYv12
#
# 2) Run all samples:
#   python3 runAll.py
#------------------------------------------------------------------------------

import os
import argparse
import subprocess
from datetime import datetime, timedelta

SAMPLES = {
    "dyv9": {
        "input": "inputs/DY_NanoAODv9.root",
        "output": "outputs/hst_DY_NanoAODv9.root",
        "era": "2018",
        "sample": "DYtoLL",
    },
    "dyv12": {
        "input": "inputs/DY_NanoAODv12.root",
        "output": "outputs/hst_DY_NanoAODv12.root",
        "era": "2022-postEE",
        "sample": "DYJetsToLL",
    },
    "qcdv12": {
        "input": "inputs/QCD_NanoAODv12.root",
        "output": "outputs/hst_QCD_NanoAODv12.root ",
        "era": "2022-postEE",
        "sample": "QCD",
    },
    "dyv15": {
        "input": "inputs/DY_NanoAODv15.root",
        "output": "outputs/hst_DY_NanoAODv15.root",
        "era": "2024",
        "sample": "DYJetsTo2L",
    },
    "datav13": {
        "input": "inputs/data_Muon0_2023C.root",
        "output": "outputs/hst_data_Muon0_2023C.root",
        "era": "2023-preBPix",
        "sample": "Muon",
    },
}

HLINE = "-"*75

def main():
    start = datetime.now()
    
    parser = argparse.ArgumentParser()
    parser.add_argument("--sample", choices=SAMPLES.keys(), help="Sample to process. Else, process all.")
    args = parser.parse_args()
    if args.sample:
        processOneSample(SAMPLES[args.sample])
    else:
        for config in SAMPLES.values():
            processOneSample(config)

    ## Print time:
    elapsed = int((datetime.now() - start).total_seconds())
    hours, remainder = divmod(elapsed, 3600)
    minutes, seconds = divmod(remainder, 60)
    print(HLINE)
    print(f">> Total runtime: \033[33;1m{hours:02d}:{minutes:02d}:{seconds:02d}\033[0m")

def processOneSample(config):
    if not os.path.isfile(config["input"]):
        print(f"\033[31m[ERROR] Input file not found: {config['input']}\033[0m")
        return
    
    command = [
        "root", "-l", "-q",
        "compile_and_run.C("
        f'"{config["input"]}",'
        f'"{config["output"]}",'
        f'"{config["era"]}",'
        f'"{config["sample"]}"'
        ")"
    ]
    print(HLINE)
    print(">> \033[33m", end="")
    print(" ".join(command) + "\033[0m")
    subprocess.run(command, check=True)

if __name__ == "__main__": main()
