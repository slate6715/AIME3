# AIME3
Advanced Interative Mud Engine - 3 - code engine designed to run old-fashioned text adventures.

# AIME3
Advanced Interative Mud Engine - 3 - code engine designed to run old-fashioned text adventures.

## AIME3 with Docker
Please note to run any of the below commands you must have [Docker](https://www.docker.com) installed. You can find getting started guides for your preferred OS [here](https://docs.docker.com/get-started/).  Please note at this time AIME v3 has not been published to the Docker Hub so you will need to build from scratch.

### Building container from scratch.

From the root of the AIME v3 project directory:

`docker build -t aime3/aime3 .`

### Running the Docker container
By default AIME v3 runs on port `6715` inside the container image.  You can change the external port by modifying the port number to the left of the `:` inside the `-p` arg.

#### Default AIME v3 Configuration
`docker run -d -p 6715:6715 aime/aime3`

#### Running with a custom port
`docker run -d -p 4000:6715 aime/aime3`

### Persisting the data directory
Anything inside of a Docker container is volatile by default.  A volume mount point has been provided to persist the data directory through container lifecycles. Inside of the container the data directory lives at `/opt/aime3/data`.

`docker run -d -p 6715:6715 -v /path/to/host/datadir:/opt/aime3/data aime/aime3`

**Note:** The container image also has the tools required to build AIME v3 in it as well as an `src` directory mount available.  If you would like to tinker with the source code and persist your changes you can like so:

```bash
docker run -p 6715:6715 \
-v /path/to/host/datadir:/opt/aime3/data \
-v /path/to/host/srcdir:/opt/aime3/src \
aime/aime3 /bin/bash
```

