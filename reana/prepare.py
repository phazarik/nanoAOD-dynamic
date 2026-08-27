#!/usr/bin/env python3

# ------------------------------------------------------------------------------
# REANA WORKFLOW PREPARATION
#
# This script is run once on lxplus before submitting the workflow to REANA.
# It converts the local sample registry (samples.json) into one self-contained
# JSON file per sample under reana/samples/. Each generated file contains the
# full list of input files together with the analysis parameters (era, sample
# name, output filename) needed to process that sample.
#
# samples.json itself is only a local, prepare-time index used to know which
# datasets to query and how to label them. It is never uploaded to REANA, and
# the rest of the workflow (Snakefile, runJob.py) has no knowledge of it -
# only the generated per-sample JSON files matter from that point on. This
# makes it straightforward to add a small hand-written sample file (e.g.
# test_sample.json) directly under reana/samples/ for debugging, without
# touching samples.json or re-running a DAS query at all.
#
# The script:
#   1. Checks for a valid CMS grid proxy and requests one if necessary.
#   2. Reads the sample registry from samples.json.
#   3. For DAS-based samples, queries DAS for the list of files belonging to
#      the dataset. For local samples, the file list is taken as-is from
#      samples.json.
#   4. Writes one JSON file per sample to reana/samples/, combining the file
#      list with the sample's processing parameters.
# ------------------------------------------------------------------------------

import json
import subprocess
import sys
from pathlib import Path
RED, YELLOW, RESET, BOLD = "\033[31m", "\033[33m", "\033[0m", "\033[1m"

## Global XRootD redirector used to access CMS files from the REANA jobs.
## Only applied to DAS-sourced files; local files are already given as
## final, usable paths in samples.json.
REDIRECTOR = "root://cms-xrd-global.cern.ch/"

## Resolve paths relative to the location of this script rather than the
## directory from which it is executed.
## This ensures that the generated sample files are always written to the
## repository's reana/samples/ directory.
REANA_DIR = Path(__file__).resolve().parent
REPO_ROOT = REANA_DIR.parent
SAMPLES_JSON = REANA_DIR / "samples.json"
SAMPLES_OUT_DIR = REANA_DIR / "samples"

def main():
    ensure_grid_proxy()
    build_sample_files()
    print("\n>> Sample files are ready in reana/samples/.")

def ensure_grid_proxy():
    print(">> Checking for a valid grid proxy ...")
    result = subprocess.run(["voms-proxy-info", "--exists", "--valid", "1:00"], capture_output=True)
    if result.returncode == 0: return
    print(YELLOW + ">> No valid proxy found, requesting one for the CMS VO ..." + RESET)
    subprocess.run(["voms-proxy-init", "--voms", "cms", "--valid", "168:00"], check=True)

def build_sample_files():
    ## The sample registry lists every dataset that needs to be turned into
    ## a per-sample JSON file, together with its processing parameters.
    if not SAMPLES_JSON.is_file(): sys.exit(RED + f"[ERROR] {SAMPLES_JSON} is missing!" + RESET)
    with open(SAMPLES_JSON) as f: samples = json.load(f)
    SAMPLES_OUT_DIR.mkdir(parents=True, exist_ok=True)

    print(f">> Building per-sample files from {SAMPLES_JSON} ...")
    for key, config in samples.items():
        source = config.get("source", "das")
        if source == "das":
            dataset = config["dataset"]
            print(f"   - {key}: querying DAS for {dataset}")
            files = query_das(dataset)
        else:
            ## Local samples list their files directly in samples.json;
            ## these are taken as-is, without the XRootD redirector prefix.
            files = config["files"]

        parameters = {
            "sample": config["sample"],
            "era": config["era"],
            "output": config["output"],
        }
        write_sample_file(SAMPLES_OUT_DIR / f"{key}.json", files, parameters)

def query_das(dataset):
    ## Use dasgoclient to retrieve the logical file names (LFNs) belonging
    ## to the requested CMS dataset, then prefix each with the XRootD
    ## redirector so the resulting paths can be passed directly to ROOT.
    result = subprocess.run(
        ["dasgoclient", "--query", f"file dataset={dataset}"],
        capture_output=True,
        text=True,
        check=True
    )
    lfns = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    return [REDIRECTOR + lfn for lfn in lfns]

def write_sample_file(path, files, parameters):
    payload = {"files": files, "parameters": parameters}
    with open(path, "w") as out: json.dump(payload, out, indent=2)
    print(f">> Wrote {YELLOW}{BOLD}{path}{RESET} ({len(files)} files)")

if __name__ == "__main__": main()
