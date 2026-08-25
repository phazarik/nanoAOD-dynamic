#!/bin/bash
# ------------------------------------------------------------------------------
# This is the master wrapper script executed by HTCondor on the remote grid
# worker node. It executes the ROOT macro by providing the right parameters.
# ------------------------------------------------------------------------------

# -------------------
# 1. Parse Arguments
# -------------------
# CRAB passes arguments (defined in crab_config.py) as key=value pairs.
# This loop dynamically exports them as environment variables.
for ARGUMENT in "$@"
do
    if [[ "$ARGUMENT" == *"="* ]]; then
        KEY=$(echo $ARGUMENT | cut -f1 -d=)
        VALUE="${ARGUMENT#*=}"
        export "$KEY"="$VALUE"
    fi
done

# -------------------------
# 2. Extract the workspace
# -------------------------
# The entire parent directory is shipped as sandbox.tar.gz.
# Extracting it ensures ROOT has access to all headers and trained models.
echo ">> Extracting workspace..."
tar -xzf sandbox.tar.gz

# ------------------------
# 3. Fetch the input file
# ------------------------
# CRAB injected the assigned input file into PSet.py. The following runs a python
# command to print that filename and store it in the $infile variable.
infile=$(python3 -c "import PSet; print(PSet.process.source.fileNames[0])")

# -----------------------------------------------------------------------------
# 4. Execute the analysis
# -----------------------------------------------------------------------------
# Define the output file. This MUST exactly match the name specified 
# in crab_config.py under config.JobType.outputFiles.
outfile="output.root"

# Prepend the XRootD redirector to stream the dataset over the grid network.
root_command="compile_and_run.C(\"root://cms-xrd-global.cern.ch//$infile\", \"$outfile\", \"$era\", \"$sample\")"

echo ">> Running ROOT command: root -q -b -l '$root_command'"

# Execute ROOT in batch mode (-b), quit when finished (-q), and suppress splash screen (-l).
root -q -b -l "$root_command"

# IMMEDIATELY capture the exit code of the ROOT process.
# Running any other command before this will overwrite the $? variable.
ROOT_EXIT_CODE=$?

# -----------------------------------------------------------------------------
# 5. Generate the mandatory FrameworkJobReport.xml (FJR)
# -----------------------------------------------------------------------------
# CRAB requires this XML file to determine job status.

if [ $ROOT_EXIT_CODE -eq 0 ]; then
    # SUCCESS (Exit Code 0): Generate an empty FJR.
    # CRAB marks the job as SUCCESS and stages out the output file.
    cat << EOF > FrameworkJobReport.xml
<FrameworkJobReport>
</FrameworkJobReport>
EOF

else
    # FAILURE (Non-Zero Exit Code): Write the <FrameworkError> tag.
    # This ensures the exact error surfaces on the CRAB dashboard for proper retries.
    cat << EOF > FrameworkJobReport.xml
<FrameworkJobReport>
    <FrameworkError ExitStatus="${ROOT_EXIT_CODE}" Type="Wrapper exit code"/>
</FrameworkJobReport>
EOF
fi

# -----------------------------------------------------------------------------
# 6. Pass the exit code back to HTCondor
# -----------------------------------------------------------------------------
# The bash wrapper script must terminate with the exact ROOT exit code.
# Otherwise, it defaults to 0, masking any actual failures from HTCondor.
exit $ROOT_EXIT_CODE
