#!/usr/bin/env python3
import os, argparse

parser = argparse.ArgumentParser()
parser.add_argument("--all", action="store_true", help="Remove analysis output files (*.root, *.txt)")
args = parser.parse_args()

script_dir = os.path.dirname(os.path.abspath(__file__))

## Default cleanup: Editor backups and ROOT ACLiC compilation artifacts
default_junk = [
    "*~", 
    "*.so", 
    "*.d", 
    "*.pcm", 
    "*_ACLiC_*"
]

for f in default_junk:
    cmd = f"rm -rf {os.path.join(script_dir, f)}"
    print(f">> {cmd}")
    os.system(cmd)

## --all cleanup: Output ROOT files and text summaries
if args.all:
    all_junk = [
        "outputs/*.root", 
        "outputs/*.txt",
        "*.root",
        "*.txt"
    ]
    for f in all_junk:
        cmd = f"rm -rf {os.path.join(script_dir, f)}"
        print(f">> {cmd}")
        os.system(cmd)
