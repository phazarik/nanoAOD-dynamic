# ------------------------------------------------------------------------------
# CRAB requires a CMSSW ParameterSet (PSet) file to run, even if the analysis
# does not use CMSSW. This file acts as a dummy container.
#
# How it works:
#   When CRAB splits the DAS dataset into chunks, it dynamically injects the
#   list of assigned XRootD files into process.source.fileNames. Our wrapper
#   script (crab_script.sh) later reads this list using Python.
# ------------------------------------------------------------------------------

import FWCore.ParameterSet.Config as cms
process = cms.Process("NANO")

## Tell the process to run over all events in the provided files
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

## Initialize an empty list of filenames. CRAB will populate this automatically
## on the worker node.
process.source = cms.Source("PoolSource", fileNames=cms.untracked.vstring())
