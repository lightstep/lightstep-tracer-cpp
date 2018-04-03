#!/bin/bash

set -e

apt-get update 
apt-get install --no-install-recommends --no-install-suggests -y \
         curl \
         unzip

# Make sure you grab the latest version
cd /
curl -OL https://github.com/google/protobuf/releases/download/v3.2.0/protoc-3.2.0-linux-x86_64.zip

# Unzip
unzip protoc-3.2.0-linux-x86_64.zip -d protoc3

# Move only protoc* to /usr/bin/
sudo mv protoc3/bin/protoc /usr/bin/protoc
