# Run the setup with REANA

### What is REANA?
REANA is a tool made at CERN to run physics analyses in the cloud. Instead of running heavy scripts directly on an lxplus terminal, REANA takes the code and runs it on CERN's big computing clusters.

*   **What is running in the background?** It uses containers (like Docker) to bundle up all the required ROOT/CMS software, and then sends the heavy computing jobs to batch systems like HTCondor.
*   **Is it like GitLab CI/CD?** Kind of! Both use a YAML file to automate steps. But while GitLab CI is mostly just for testing software, REANA is built specifically to crunch massive datasets and manage long-running physics jobs.
*   **Read more:** [https://docs.reana.io/](https://docs.reana.io/)

### Prerequisites
*   Access to CERN lxplus.
*   Access to CMS DAS (a valid grid certificate).

---

##  Setting up REANA [one-time setup]

First, a command-line access token is needed to talk to the servers. 
1. Go to [https://reana.cern.ch/](https://reana.cern.ch/) and log in. (This site also acts as a dashboard to check on jobs later).
2. Request access. (Someone has to approve this, so it might take a few hours).
3. Once approved, grab the access token from the web profile.

Next, create a local `.env` file inside the `reana/` folder. **Never commit or push this file to Git.**
```bash
export REANA_SERVER_URL=[https://reana.cern.ch](https://reana.cern.ch)
export REANA_ACCESS_TOKEN=XXXXXXXXXXXXXXXXXXXXXXX
```
Source this file so the terminal knows the token. (This can also just be added to a `.bashrc`). Now, activate the Python environment and load the tokens:
```bash
source /afs/cern.ch/user/r/reana/public/reana/bin/activate
source .env
```
Check if the connection is working:
```bash
reana-client ping
```
To let REANA read CMS data and write the final results back to EOS, it needs some saved credentials. These only need to be uploaded once.

### Configure DAS/XRootD access (grid proxy)

To read datasets over xrootd, upload the grid certificate and its password. Replace `<GRID_PASSPHRASE>` with the actual password (the same one used when running `voms-proxy-init`).

```bash
read -s -p "Enter Grid Passphrase: " MY_PASS
reana-client secrets-add --file ~/.globus/userkey.pem --file ~/.globus/usercert.pem --env VOMSPROXY_PASS=$(echo -n "$MY_PASS" | base64) --env VONAME=cms --overwrite
unset MY_PASS
```

### Configure EOS Access (Kerberos)

To save output files directly into an EOS folder, generate a Kerberos "keytab". On `lxplus`, use the dedicated CERN utility to create this file securely:
```bash
cern-get-keytab --keytab mykeytab.keytab --user --login phazarik
```
Next, test the freshly generated keytab locally to ensure it works before uploading it to the cloud:
```bash
kdestroy
kinit -kt mykeytab.keytab phazarik@CERN.CH
klist
```
Now, upload the keytab to REANA. Notice that the `CERN_KEYTAB` environment variable is explicitly set so the workflow engine knows exactly what filename to look for:
```bash
reana-client secrets-add \
  --env CERN_USER=phazarik \
  --env CERN_KEYTAB=mykeytab.keytab \
  --file mykeytab.keytab \
  --overwrite
```
Finally, delete the local file to keep the workspace secure:
```bash
rm mykeytab.keytab
```
**What just happened?** A keytab is essentially a locked file holding the CERN password. By uploading it as a secret, REANA can automatically renew tickets in the background to authenticate as `phazarik`. This way, a job can finish at 3 AM and safely write its root files to EOS without asking for a password. The local file is deleted to keep things secure.

## How the analysis is configured

Here is a quick look at the files inside the `reana/` directory:
```
nanoAOD-dynamic
├── reana_prepare.py
├── reana_runjob.py
├── reana_samples.json
├── reana.yaml
└── Snakefile
```
-   **`reana_prepare.py`**: A quick script meant to be run locally. It talks to DAS to find all the files for a dataset and saves their URLs in text files.
    
-   **`reana_samples.json`**: A cheat sheet that maps a dataset name to its file list, physics parameters (like the era), and what the final output file should be named.
    
-   **`reana_runjob.py`**: A lightweight Python wrapper. It is executed by the workflow on the compute nodes to read the JSON configuration, parse the exact file chunk, and safely execute the ROOT macro.
    
-   **`Snakefile`**: This is the mosThe recipe book. It builds the workflow dependency graph, telling REANA how to dispatch the parallel jobs, merge the resulting histograms, and copy the final results to EOS.
    
-   **`reana.yaml`**: The main config file. It tells REANA which files and directories to ship to the cloud and points it to the Snakefile to start the work.

## Running the analysis

When the code is ready to run or new datasets are added, just follow these steps.

1. Activate the environment and load the access tokens.
	 ```bash
	 source /afs/cern.ch/user/r/reana/public/reana/bin/activate
	 source .env
	 reana-client ping
	 ```
2. Run the Python script locally to gather all the file URLs from DAS.
	```bash
	python3 prepare.py
	```
3. Upload the code and start the run. The `-w` flag gives the run a name.
	```bash
	reana-client run -w nanoaod-run
	```

Monitor progress and check logs:
```bash
reana-client status -w nanoaod-run
reana-client logs -w nanoaod-run
```
