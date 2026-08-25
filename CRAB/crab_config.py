# ------------------------------------------------------------------------------
# This file defines the parameters for a CRAB task (computing resources,
# input datasets, output destinations, etc.).
#
# Instead of hardcoding values for one dataset, the arguments to the shell
# executableare assigned via environment variables passed by submitJobs.py.
# This is convenient for bulk submission.
# ------------------------------------------------------------------------------

from WMCore.Configuration import Configuration
from CRABClient.UserUtilities import config
import os

# ------------------------------------------------------------------------------
# Read dynamic parameters set by submitJobs.py
# ------------------------------------------------------------------------------
username = os.getenv("USER")
era     = os.getenv('CRAB_ERA')
sample  = os.getenv('CRAB_SAMPLE')
reqname = os.getenv('CRAB_REQNAME')
dataset = os.getenv('CRAB_DATASET')
outdir  = os.getenv('CRAB_OUTDIR') ## Full EOS path

config = config()

# ------------------------------------------------------------------------------
# General configuration: defines where local logs and status caches are stored.
# ------------------------------------------------------------------------------
config.General.transferOutputs = True
config.General.transferLogs = True
config.General.requestName = reqname
config.General.workArea = 'submitted'

# ------------------------------------------------------------------------------
# JobType configuration: defines what to run and what to send to the worker node.
# Recommended: send all the code/headers as sandbox.tar.gz
# ------------------------------------------------------------------------------
config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'PSet.py'
config.JobType.scriptExe  = 'crab_script.sh'
config.JobType.scriptArgs = [f"era={era}", f"sample={sample}"]
config.JobType.inputFiles = ['sandbox.tar.gz']
config.JobType.outputFiles = ['output.root']

# ------------------------------------------------------------------------------
# Data configuration: defines how the dataset is split.
# Assigning 1 ROOT file per job makes the no. of outputs match with DAS files.
# outLFNDirBase dictates the final EOS destination path.
# ------------------------------------------------------------------------------
config.Data.inputDataset = dataset
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.outLFNDirBase = outdir ## Full EOS path
config.Data.publication = False
config.Data.outputDatasetTag = reqname

# ------------------------------------------------------------------------------
# Site configuration
# ------------------------------------------------------------------------------
config.Site.storageSite = 'T3_CH_CERNBOX'
config.Site.blacklist   = ['T2_FR_GRIF', 'T3_UK_ScotGrid_GLA', 'T1_IT_CNAF']
