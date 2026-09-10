FROM ubuntu:22.04

RUN apt-get update

# Install build tools
RUN apt-get install -yy g++ cmake

# Install dependencies
RUN apt-get install -yy libxml2-dev nlohmann-json3-dev python3-dev python3-venv pybind11-dev pybind11-json-dev libpapi-dev hwloc libhwloc-common libhwloc-dev
