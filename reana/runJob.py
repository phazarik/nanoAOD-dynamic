#!/usr/bin/env python3

# ------------------------------------------------------------------------------
# REANA JOB WRAPPER (Chunked Edition)
#
# This script executes the analysis for a CHUNK of (sample, input-file) pairs.
# It is called by the Snakefile's process_chunk rule, with the sample name,
# chunk index, chunk size, and final output filename passed as arguments.
#
# The script:
#   1. Reads the sample configuration from samples.json.
#   2. Determines the subset (chunk) of input NanoAOD files to process.
#   3. Retrieves the analysis era and sample name.
#   4. Runs the ROOT analysis using compile_and_run.C for each file sequentially.
#   5. Merges the temporary ROOT files into a single chunk output file.
# ------------------------------------------------------------------------------

import sys
import os
import json
import subprocess

def main():
    # Arguments are provided by the process_chunk rule in the Snakefile:
    #   argv[1] = sample key
    #   argv[2] = index of the chunk
    #   argv[3] = number of files per chunk
    #   argv[4] = output ROOT file for this chunk
    sample_key = sys.argv[1]
    chunk_index = int(sys.argv[2])
    chunk_size = int(sys.argv[3])
    final_outfile = sys.argv[4]

    # The workflow is executed from the repository root, so the sample
    # configuration can be accessed using this relative path.
    if not os.path.isfile("reana/samples.json"):
        sys.exit("[ERROR] reana/samples.json not found.")

    with open("reana/samples.json", "r") as f:
        config = json.load(f)[sample_key]

    # Determine the input NanoAOD files.
    # For samples using DAS, the filelist contains one input file per line.
    # For local samples, the input file is specified directly in samples.json.
    source = config.get("source", "local")

    if source == "das":
        if not os.path.isfile(config["filelist"]):
            sys.exit(f"[ERROR] Filelist {config['filelist']} not found.")
        with open(config["filelist"], "r") as f:
            all_files = [line.strip() for line in f if line.strip()]
    else:
        all_files = [config["input"]]

    # Slice the filelist to get only the files assigned to this specific chunk
    start_idx = chunk_index * chunk_size
    end_idx = start_idx + chunk_size
    chunk_files = all_files[start_idx:end_idx]

    # Read the analysis configuration associated with this sample.
    era = config["era"]
    sample = config["sample"]

    temp_outputs = []

    # Run ROOT analysis on each file in the chunk sequentially
    for i, infile in enumerate(chunk_files):
        # Give each file a unique temporary name so they don't overwrite each other
        temp_outfile = f"temp_out_{i}.root"
        temp_outputs.append(temp_outfile)

        # Build the ROOT command that runs the analysis.
        # compile_and_run.C takes the input file, output file, data-taking era,
        # and sample name as arguments.
        cmd = (
            f"root -l -b -q "
            f"'compile_and_run.C(\"{infile}\", \"{temp_outfile}\", \"{era}\", \"{sample}\")'"
        )
        print(f"Executing [{i+1}/{len(chunk_files)}]: {cmd}")
        
        # Run the ROOT analysis and stop the job if ROOT returns an error.
        subprocess.run(cmd, shell=True, check=True)

    # Merge temporary chunk files, or rename if there's only one.
    # This ensures the Snakemake target file is created correctly.
    if len(temp_outputs) > 1:
        print(f"Merging {len(temp_outputs)} temporary files into {final_outfile}...")
        merge_cmd = f"hadd -f {final_outfile} " + " ".join(temp_outputs)
        subprocess.run(merge_cmd, shell=True, check=True)
        
        # Clean up temporary root files using the os module
        for temp_file in temp_outputs:
            if os.path.isfile(temp_file):
                os.remove(temp_file)
    elif len(temp_outputs) == 1:
        print(f"Renaming single output to {final_outfile}...")
        subprocess.run(f"mv {temp_outputs[0]} {final_outfile}", shell=True, check=True)
    else:
        print("No files were processed.")

if __name__ == "__main__": main()
