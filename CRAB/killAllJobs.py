#!/usr/bin/env python3
# ------------------------------------------------------------------------------
# Emergency stop script. It recursively scans a target directory (default:
# "submitted") for active CRAB tasks and issues a kill command to cancel
# all running jobs on the CMS grid.
# ------------------------------------------------------------------------------

import os
import argparse
import subprocess
RED, YELLOW = "\033[31m","\033[33m"
RESET,BOLD,DIM = "\033[0m","\033[1m","\033[2m"

def main():
    parser = argparse.ArgumentParser(description="Kill all running CRAB jobs in a target directory.")
    parser.add_argument("-d", default="submitted", help="Target directory (default: 'submitted')")
    args = parser.parse_args()

    subdir = args.d
    if not os.path.exists(subdir):
        print(f"{RED}[ERROR] Directory '{subdir}' not found.{RESET}")
        return

    print(f">> Scanning '{subdir}' for CRAB jobs to kill...\n")

    jobs_found = False
    for root, dirs, files in os.walk(subdir):

        ## Identify a valid CRAB job directory using standard file signatures or prefixes
        is_crab_dir = (
            ".requestcache" in files
            or "crab.log" in files
            or (os.path.basename(root).startswith("crab_") and root != subdir)
        )

        ## If it's not a CRAB directory, keep walking through the directory tree
        if not is_crab_dir: continue

        jobs_found = True
        print(f">> Removing job: {YELLOW}{os.path.relpath(root, subdir)}{RESET}")
        
        ## Use subprocess instead of os.system to capture output and hide HTML errors
        result = subprocess.run(["crab", "kill", "-d", root], capture_output=True, text=True)
        output = result.stdout + result.stderr
        
        if "400 Bad Request" in output or "This request are invalid" in output:
            print(f"   {RED}[ERROR] Server returned 400 Bad Request.{RESET}")
            print(f"   {DIM}The job is likely in SUBMITREFUSED state and cannot be killed because it was never submitted.{RESET}")
            print(f"   {DIM}You can safely clean it up manually: rm -rf {root}{RESET}\n")
        else:
            ## Print the normal output, indented and formatted
            clean_output = "\n   ".join(output.strip().split("\n"))
            print(f"{DIM}   {clean_output}{RESET}\n")

        dirs.clear() ## prevent os.walk to walk further in

    if not jobs_found: print(f">> No CRAB job directories found in '{subdir}'")
    print("Done!")

if __name__ == "__main__": main()
