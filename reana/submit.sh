#!/bin/bash
# ------------------------------------------------------------------------------
# One-shot helper to submit a REANA workflow with a fresh VOMS proxy each time.
#
# What it does, in order:
#   1. Destroys any existing local VOMS proxy for this user (if present).
#   2. Generates a brand-new proxy from the CERN grid cert/key, valid for
#      up to 96h (actual granted lifetime depends on the CMS VOMS server's
#      policy for this account).
#   3. Uploads that proxy file to REANA as a secret, so the htcondorcern
#      worker jobs can pick it up (see runJob.py, which locates the staged
#      proxy file and exports X509_USER_PROXY from it).
#   4. Submits the given workflow.
#
# Requirements:
#   - ~/.globus/usercert.pem and ~/.globus/userkey.pem must exist
#     and must be readable only by user (chmod 600).
#   - Grid passphrase for voms-proxy-init.
#   - reana-client must already be configured (activate reana env)
#
# Example"
#   ./submit_reana.sh nanoaod-dynamic-proxy
# ------------------------------------------------------------------------------

set -e
if [ -z "$1" ]; then
    echo "Usage: $0 <workflow-name>"
    exit 1
fi

WORKFLOW_NAME="$1"
PROXY=/tmp/x509up_u$(id -u) ## standard default path voms-proxy-init writes to
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

## destroy any existing proxy, always start fresh
if [ -f "$PROXY" ]; then
    echo ">> Destroying existing proxy at $PROXY ..."
    voms-proxy-destroy --file "$PROXY" 2>/dev/null || rm -f "$PROXY"
fi

## generate a fresh proxy (will prompt for Grid passphrase)
echo ">> Generating fresh VOMS proxy..."
voms-proxy-init --cert ~/.globus/usercert.pem \
                 --key ~/.globus/userkey.pem \
                 --voms cms --valid 96:00

## upload the fresh proxy to REANA as a secret
#echo ">> Uploading proxy as REANA secret..."
#reana-client secrets-add --env VONAME=cms \
#             --env VOMSPROXY_FILE=$(basename "$PROXY") \
#             --file "$PROXY" \
#             --overwrite

# Copy proxy to repo root so REANA uploads it as a normal workspace file
echo ">> Staging proxy into workspace..."
cp "$PROXY" "$REPO_ROOT/proxy.pem"

## submit the workflow from the root directory where reana.yaml sits
echo ">> Submitting workflow: $WORKFLOW_NAME (from $REPO_ROOT) ..."
(cd "$REPO_ROOT" && reana-client run -w "$WORKFLOW_NAME")
