#!/usr/bin/env python3

# ------------------------------------------------------------------------------
# REANA WORKFLOW PREPARATION
#
# This script is run once on lxplus before submitting the workflow to REANA.
# It prepares the input filelists for samples that use DAS as their source.
#
# The script:
#   1. Checks for a valid CMS grid proxy and requests one if necessary.
#   2. Reads the sample configuration from samples.json.
#   3. Queries DAS for the files belonging to each DAS-based sample.
#   4. Writes the resulting filelists to reana/filelists/.
#
# The generated filelists are then included in the REANA workflow and used by
# the Snakefile to determine which input files need to be processed.
# ------------------------------------------------------------------------------

import json
import subprocess
import sys
from pathlib import Path
RED, YELLOW, RESET, BOLD = "\033[31m", "\033[33m", "\033[0m", "\033[1m"

## Global XRootD redirector used to access CMS files from the REANA jobs.
REDIRECTOR = "root://cms-xrd-global.cern.ch/"

## Resolve paths relative to the location of this script rather than the
## directory from which it is executed.
## This ensures that the generated filelists are always written to the
## repository's reana/filelists/ directory.
REANA_DIR = Path(__file__).resolve().parent
REPO_ROOT = REANA_DIR.parent
SAMPLES_JSON = REANA_DIR / "samples.json"

def main():
    ensure_grid_proxy()
    build_filelists()
    print("\n>> Filelists are ready.")

def ensure_grid_proxy():
    print(">> Checking for a valid grid proxy ...")
    result = subprocess.run(["voms-proxy-info", "--exists", "--valid", "1:00"], capture_output=True)
    if result.returncode == 0: return
    print(YELLOW+ ">> No valid proxy found, requesting one for the CMS VO ..."+ RESET)
    subprocess.run(["voms-proxy-init", "--voms", "cms", "--valid", "168:00"], check=True)

def build_filelists():
    ## The sample configuration is required to determine which datasets
    ## should be queried and where their filelists should be written.
    if not SAMPLES_JSON.is_file():
        sys.exit(RED + f"[ERROR] {SAMPLES_JSON} is missing!" + RESET)
    with open(SAMPLES_JSON) as f: samples = json.load(f)

    ## Only samples with source="das" require a DAS query.
    ## Samples using local input files are handled directly by the Snakefile.
    das_samples = {key: cfg for key, cfg in samples.items()
                   if cfg.get("source") == "das"}

    print(f">> Building filelists from {SAMPLES_JSON} ...")
    for key, config in das_samples.items():
        ## Filelist paths in samples.json are relative to the repository root.
        ## Resolve them here so the generated files are written to the correct
        ## location regardless of the current working directory.
        dataset = config["dataset"]
        filelist_path = REPO_ROOT / config["filelist"]
        print(f"   - {key}: querying DAS for {dataset}")
        files = query_das(dataset)
        write_filelist(filelist_path, files)

def query_das(dataset):
    ## Use dasgoclient to retrieve the logical file names (LFNs) belonging
    ## to the requested CMS dataset.
    result = subprocess.run(
        ["dasgoclient", "--query", f"file dataset={dataset}"],
        capture_output=True,
        text=True,
        check=True
    )
    ## Remove empty lines and surrounding whitespace from the DAS output.
    return [line.strip()
            for line in result.stdout.splitlines()
            if line.strip()]

def write_filelist(path, files):
    ## Prefix each CMS logical file name with the global XRootD redirector.
    ## The resulting paths can be passed directly to ROOT for remote access.
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w") as out:
        for lfn in files:
            out.write(REDIRECTOR + lfn + "\n")
    print(f">> Wrote {YELLOW}{BOLD}{path}{RESET} ({len(files)} files)")

if __name__ == "__main__": main()
