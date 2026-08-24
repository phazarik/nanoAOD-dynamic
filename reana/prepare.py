#!/usr/bin/env python3
# ----------------------------------------------------------------------------------
# One-time setup, run on lxplus before the workflow is submitted to REANA.
#
#   1. Makes sure a grid proxy is available (needed to query DAS).
#   2. Builds a filelist (one xrootd URL per line) for every DAS-backed
#      sample declared in samples.json.
#
# Filelist paths in samples.json (e.g. "filelists/dyv12.txt") are relative
# to the repository root, matching how the Snakefile reads them, so they
# are written there rather than inside reana/.
# ----------------------------------------------------------------------------------

import json
import subprocess
import sys
from pathlib import Path
RED, YELLOW, RESET, BOLD = "\033[31m", "\033[33m", "\033[0m", "\033[1m"

REDIRECTOR = "root://cms-xrd-global.cern.ch/"
REANA_DIR = Path(__file__).resolve().parent
REPO_ROOT = REANA_DIR.parent
SAMPLES_JSON = REANA_DIR / "samples.json"

def main():
    ensure_grid_proxy()
    build_filelists()
    print("\n>> Filelists are ready.")
    print_secrets_reminder()

def ensure_grid_proxy():
    print(">> Checking for a valid grid proxy ...")
    result = subprocess.run(
        ["voms-proxy-info", "--exists", "--valid", "1:00"],
        capture_output=True
    )
    if result.returncode == 0: return
    print(YELLOW + ">> No valid proxy found, requesting one for the CMS VO ..." + RESET)
    subprocess.run(["voms-proxy-init", "--voms", "cms", "--valid", "168:00"], check=True)

def build_filelists():
    if not SAMPLES_JSON.is_file(): sys.exit(RED + f"[ERROR] {SAMPLES_JSON} is missing!" + RESET)
    with open(SAMPLES_JSON) as f: samples = json.load(f)
    das_samples = {key: cfg for key, cfg in samples.items() if cfg.get("source") == "das"}
    print(f">> Building filelists from {SAMPLES_JSON} ...")
    for key, config in das_samples.items():
        dataset = config["dataset"]
        filelist_path = REPO_ROOT / config["filelist"]
        print(f"   - {key}: querying DAS for {dataset}")
        files = query_das(dataset)
        write_filelist(filelist_path, files)

def query_das(dataset):
    result = subprocess.run(
        ["dasgoclient", "--query", f"file dataset={dataset}"],
        capture_output=True, text=True, check=True
    )
    return [line.strip() for line in result.stdout.splitlines() if line.strip()]

def write_filelist(path, files):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w") as out:
        for lfn in files:
            out.write(REDIRECTOR + lfn + "\n")
    print(f"     wrote {YELLOW}{BOLD}{len(files)}{RESET} files to {path}")

def print_secrets_reminder():
    print()
    print(">> Two sets of REANA secrets are still needed before the workflow")
    print("   can run (only once, not every time this script runs):")
    print()
    print("   # For reading the datasets over xrootd:")
    print("   reana-client secrets-add --file userkey.pem --file usercert.pem \\")
    print("       --env VOMSPROXY_PASS=$(echo -n '<grid-passphrase>' | base64) \\")
    print("       --env VONAME=cms")
    print()
    print("   # For writing outputs to EOS:")
    print("   reana-client secrets-add --file keytab --env CERN_USER=<username>")

if __name__ == "__main__": main()
