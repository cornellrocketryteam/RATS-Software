# RATS Software Proxy
Rotational Antenna Tracking System (RATS) Software for the Cornell Rocketry Team

[Confluence Page](https://confluence.cornell.edu/display/crt/RATS+Software) 


## How to Use
On an x86 computer, run the RATS proxy by building the Dockerfile in the project directory, and then running the produced docker image.

```
docker build -t rats-proxy:latest .
docker run --init -it --rm  --privileged  rats-proxy:latest  bash
```
