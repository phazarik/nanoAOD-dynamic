#!/usr/bin/env python3

# ------------------------------------------------------------------------------
# REANA JOB WRAPPER
#
# This script executes the analysis for a CHUNK of (sample, input-file) pairs.
# It is called by the Snakefile's process_chunk rule, with the sample key,
# chunk index, chunk size, and final output filename passed as arguments.
#
# The script:
#   1. Reads the sample's own JSON file from reana/samples/<sample_key>.json,
#      which contains the full input file list and the processing parameters
#      (era, sample name) for that sample.
#   2. Determines the subset (chunk) of input NanoAOD files to process.
#   3. Runs the ROOT analysis using compile_and_run.C for each file sequentially.
#   4. Merges the temporary ROOT files into a single chunk output file.
# ------------------------------------------------------------------------------

import os, sys
import glob
import json
import subprocess
SAMPLES_DIR = "reana/samples"

def main():
    ## Arguments are provided by the process_chunk rule in the Snakefile:
    ##   argv[1] = sample key
    ##   argv[2] = index of the chunk
    ##   argv[3] = number of files per chunk
    ##   argv[4] = output ROOT file for this chunk
    sample_key = sys.argv[1]
    chunk_index = int(sys.argv[2])
    chunk_size = int(sys.argv[3])
    final_outfile = sys.argv[4]
    proxy_path = find_proxy()

    ## The workflow is executed from the repository root, so the sample
    ## file can be accessed using this relative path.
    sample_path = os.path.join(SAMPLES_DIR, f"{sample_key}.json")
    if not os.path.isfile(sample_path): sys.exit(f"[ERROR] {sample_path} not found.")
    with open(sample_path, "r") as f: sample = json.load(f)

    ## Extract parameters to the ROOT macro
    all_files   = sample["files"]
    era         = sample["parameters"]["era"]
    sample_name = sample["parameters"]["sample"]
    print(f">> Arguments to the ROOT macro loaded.")

    ## Redirecting $HOME because REANA inherits lxplus environment variables during
    ## submission, leaving $HOME pointing to AFS. Since the HTCondor worker lacks
    ## AFS write tokens, ROOT's Cling compiler hangs when silently attempting to
    ## write its cache to ~/.cache/root/. Overwriting $HOME forces ROOT to use
    ## the worker node's local scratch space instead.
    os.environ["HOME"] = os.getcwd()
    print(f">> Redirecting $HOME to: {os.getcwd()}")

    ## Prevent ONNX/OpenMP from spawning 64 threads on a 1-core Condor job.
    ## This prevents a futex deadlock during Ort::Env initialization.
    os.environ["OMP_NUM_THREADS"] = "1"
    os.environ["OMP_THREAD_LIMIT"] = "1"

    ## + following are added to strictly throttle underlying C++ math libraries
    os.environ["OPENBLAS_NUM_THREADS"] = "1"
    os.environ["MKL_NUM_THREADS"] = "1"
    os.environ["NUMEXPR_NUM_THREADS"] = "1"

    ## Force XRootD to use IPv4 to avoid silent IPv6 routing black holes 
    ## common on dual-stack grid worker nodes.
    os.environ["XRD_NETWORKSTACK"] = "IPv4"

    ## Recreate stripped ONNX symlinks on the worker node
    ## These are not uploaded by the reana configuration
    onnx_lib_dir = "onnxruntime-linux-x64-1.24.4/lib"
    so_target = "libonnxruntime.so.1.24.4"
    for link_name in ["libonnxruntime.so.1", "libonnxruntime.so"]:
        link_path = os.path.join(onnx_lib_dir, link_name)
        if not os.path.exists(link_path) and os.path.exists(os.path.join(onnx_lib_dir, so_target)):
            print(f">> Recreating symlink: {link_path} -> {so_target}")
            os.symlink(so_target, link_path) ## so_target is relative to link_path's directory

    ## Slice the file list to get only the files assigned to this specific
    ## chunk. Entries starting with "root://" are read remotely via XRootD;
    ## anything else is treated as a local path already present in the
    ## workspace (uploaded via reana.yaml).
    start_idx = chunk_index * chunk_size
    end_idx = start_idx + chunk_size
    chunk_files = all_files[start_idx:end_idx]
    temp_outputs = []

    ## Run ROOT analysis on each file in the chunk sequentially
    for i, infile in enumerate(chunk_files):

        ## Give each file a unique temporary name so they don't overwrite each other
        temp_outfile = f"temp_out_{i}.root"
        temp_outputs.append(temp_outfile)

        ## Build the ROOT command that runs the analysis.
        ## compile_and_run.C takes the input file, output file, data-taking era,
        ## and sample name as arguments.
        cmd = (
            f"root -l -b -q "
            f"'compile_and_run.C(\"{infile}\", \"{temp_outfile}\", \"{era}\", \"{sample_name}\")'"
        )
        print(f">> Executing [{i+1}/{len(chunk_files)}]: {cmd}")
        subprocess.run(cmd, shell=True, check=True)

    ## Merge temporary chunk files, or rename if there's only one.
    ## This ensures the Snakemake target file is created correctly.
    if len(temp_outputs) > 1:
        print(f">> Merging {len(temp_outputs)} temporary files into {final_outfile}...")
        merge_cmd = f"hadd -f {final_outfile} " + " ".join(temp_outputs)
        subprocess.run(merge_cmd, shell=True, check=True)

        ## Clean up temporary root files using the os module
        for temp_file in temp_outputs:
            if os.path.isfile(temp_file):
                os.remove(temp_file)

    elif len(temp_outputs) == 1:
        print(f">> Renaming single output to {final_outfile}...")
        subprocess.run(f"mv {temp_outputs[0]} {final_outfile}", shell=True, check=True)

    else: print(">> No files were processed.")

    ##clean up the proxy for safety
    if os.path.isfile(proxy_path):
        print(">> Removing proxy from worker node scratch space...")
        os.remove(proxy_path)

def find_proxy():
    proxy_path = os.path.abspath("proxy.pem")
    if not os.path.isfile(proxy_path):
        sys.exit(f"[ERROR] Proxy file not found at {proxy_path}")
    
    ## Secure the file to fix XRootD TLS warnings
    os.chmod(proxy_path, 0o600)
    os.environ["X509_USER_PROXY"] = proxy_path
    print(f"\n>> X509_USER_PROXY set to: {proxy_path}")
    return proxy_path ## Return the path for cleanup later

if __name__ == "__main__": main()
