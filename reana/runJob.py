#!/usr/bin/env python3

# ------------------------------------------------------------------------------
# REANA JOB WRAPPER
#
# This script executes the analysis for one (sample, input-file) pair.
# It is called by the Snakefile's process_file rule, with the sample name,
# input-file index, and output filename passed as command-line arguments.
#
# The script:
#   1. Reads the sample configuration from samples.json.
#   2. Determines the input NanoAOD file.
#   3. Retrieves the analysis era and sample name.
#   4. Runs the ROOT analysis using compile_and_run.C.
# ------------------------------------------------------------------------------

import sys
import json
import subprocess

def main():
    ## Arguments are provided by the process_file rule in the Snakefile:
    ##   argv[1] = sample key
    ##   argv[2] = index of the input file
    ##   argv[3] = output ROOT file
    sample_key = sys.argv[1]
    index = int(sys.argv[2])
    outfile = sys.argv[3]

    ## The workflow is executed from the repository root, so the sample
    ## configuration can be accessed using this relative path.
    with open("reana/samples.json", "r") as f: config = json.load(f)[sample_key]

    ## Determine the input NanoAOD file.
    ## For samples using DAS, the filelist contains one input file per line.
    ## The file corresponding to the current index is selected for this job.
    ## For local samples, the input file is specified directly in samples.json.
    source = config.get("source", "local")

    if source == "das":
        with open(config["filelist"], "r") as f:
            infile = [
                line.strip()
                for line in f
                if line.strip()
            ][index]
    else:
        infile = config["input"]

    ## Read the analysis configuration associated with this sample.
    era = config["era"]
    sample = config["sample"]

    ## Build the ROOT command that runs the analysis.
    ## compile_and_run.C takes the input file, output file, data-taking era,
    ## and sample name as arguments.
    cmd = (
        f"root -l -b -q "
        f"'compile_and_run.C(\"{infile}\", \"{outfile}\", \"{era}\", \"{sample}\")'"
    )
    print(f"Executing: {cmd}")

    ## Run the ROOT analysis and stop the job if ROOT returns an error.
    subprocess.run(cmd, shell=True, check=True)

if __name__ == "__main__": main()
