#!/usr/bin/env python3
# ------------------------------------------------------------------------------
# This is the main submission driver. It reads samples.json, checks the grid
# credentials, packages the analysis code into a tarball, queries DAS for
# dataset stats, and submits the CRAB jobs sequentially.
# ------------------------------------------------------------------------------
 
import os, sys
import argparse
import json
import subprocess
import ast
RED, YELLOW = "\033[31m", "\033[33m"
RESET, BOLD, DIM = "\033[0m", "\033[1m", "\033[2m"

def main():
    if not check_cmssw(): sys.exit(1) ## Needed to load some modules; not used in the analysis
    ensure_grid_proxy()

    # ------------------------------------------------
    # Define the EOS base directory here
    username = os.getenv("USER")
    eos_base = f"/store/user/{username}/crab_outputs"
    # ------------------------------------------------

    parser = argparse.ArgumentParser(description="Submit CRAB jobs in bulk.")
    parser.add_argument("--test", action="store_true", help="Run only one sample.")
    parser.add_argument("--dryrun", type=ast.literal_eval, default=True, help="Dry run. Pass True or False.")
    args = parser.parse_args()

    ## Read the input dictionary
    samples_file = "samples.json"
    if not os.path.isfile(samples_file):
        print(f"{RED}[ERROR] {samples_file} not found!{RESET}")
        sys.exit(1)
    with open(samples_file, "r") as f: samples = json.load(f)

    ## Package the code for the worker nodes
    create_sandbox(args.dryrun)

    ## Iterate through each sample defined in samples.json
    count = 0
    for key, config_dict in samples.items():
        dataset = config_dict.get("dataset")
        era = config_dict.get("era")
        sample = config_dict.get("sample")
        outdir = os.path.join(eos_base, config_dict.get("outdir", "Uncategorized"))
        if not dataset: continue

        ## Verify the dataset exists and print its statistics
        print("-" * 50)
        print(f">> [{count+1}/{len(samples)}] Querying DAS: {YELLOW}{dataset}{RESET}")
        stats = query_das(dataset)
        if not stats:
            print(f"{RED}[ERROR] Dataset missing or contains 0 files. Skipping...{RESET}")
            continue
        print(f"  - Found {stats['nfiles']} files, {stats['nevents']} events, {stats['size_gb']:.2f} GB")

        ## Jobname is decided from the key in samples.json
        reqname = f"nanoAna_{key}"
        project_dir = f"submitted/crab_{reqname}"
        if os.path.isdir(project_dir):
            print(f"{YELLOW}[WARNING] CRAB project already exists: {project_dir}. Skipping...{RESET}")
            continue

        ## Export these variables so crab_config.py can read them dynamically
        os.environ['CRAB_ERA']     = era
        os.environ['CRAB_SAMPLE']  = sample
        os.environ['CRAB_REQNAME'] = reqname
        os.environ['CRAB_DATASET'] = dataset
        os.environ['CRAB_OUTDIR']  = outdir

        print(f"  - Creating CRAB job: {reqname}")
        print(f"  - env: era    = {era}")
        print(f"  - env: sample = {sample}")
        print(f"  - env: outdir = {outdir}")
        
        ## Submit the job to CRAB
        command = "crab submit -c crab_config.py"
        if not args.dryrun: os.system(command)
        count += 1
        if args.test: break

    if not args.dryrun: print(f"\nDone! Successfully submitted {count} dataset(s).\n")
    else:
        print(f"\n[DRYRUN] Available samples = {count}")
        print(f"Submit jobs using: {YELLOW+BOLD}python3 submitJobs.py --dryrun False{RESET}\n")

# ------------------------------------------------------------------------------
def ensure_grid_proxy():
    print(">> Checking for a valid grid proxy ...")
    result = subprocess.run(["voms-proxy-info", "--exists", "--valid", "1:00"], capture_output=True)
    if result.returncode == 0: return
    print(YELLOW + ">> No valid proxy found, requesting one for the CMS VO ..." + RESET)
    subprocess.run(["voms-proxy-init", "--voms", "cms", "--valid", "168:00"], check=True)

def check_cmssw():
    if not os.environ.get("CMSSW_BASE"):
        print(f"{RED}[WARNING] No CMSSW environment detected! Please cmsenv first.{RESET}")
        return False
    return True

def create_sandbox(dryrun = False):
    # Compress the necessary C++ scripts, headers, and ONNX models from the
    # parent directory into a single sandbox tarball. This ensures the worker
    # node has the exact same directory structure as the local setup.
    print(">> Creating sandbox tarball...")
    files = "compile_and_run.C runOnce.C nanoAna.C nanoAna.h headers onnxruntime-linux-x64-1.24.4 trained_models"
    if dryrun: cmd = f"tar -cvf /dev/null -C .. {files}"
    else:      cmd = f"tar -czvf sandbox.tar.gz -C .. {files}"
    result = subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True)
    print(f"{DIM}{result.stdout}{RESET}")

def query_das(dataset):
    # Query the DAS database to fetch summary statistics (files, events, size).
    # Returns a dictionary of stats, or None if the dataset is invalid/empty.
    das_cmd = f'dasgoclient --query="summary dataset={dataset}" -json'
    das_res = subprocess.run(das_cmd, shell=True, capture_output=True, text=True)
    if das_res.returncode != 0 or not das_res.stdout.strip() or das_res.stdout.strip() == "[]": return None
    das_data = json.loads(das_res.stdout)
    if len(das_data) == 0 or "summary" not in das_data[0] or len(das_data[0]["summary"]) == 0: return None
    summary = das_data[0]["summary"][0]
    nfiles = summary.get("nfiles", 0)
    if nfiles == 0: return None
    return {
        "nfiles": nfiles,
        "nevents": summary.get("nevents", 0),
        "size_gb": summary.get("file_size", 0) / (1024**3)
    }

# ------------------------------------------------------------------------------
if __name__ == "__main__": main()
