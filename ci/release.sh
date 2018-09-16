#!/bin/bash

set -e
apt-get update 
apt-get install --no-install-recommends --no-install-suggests -y \
                wget \
                unzip

# Install ghr
cd /
wget https://github.com/tcnksm/ghr/releases/download/v0.5.4/ghr_v0.5.4_linux_amd64.zip
unzip ghr_v0.5.4_linux_amd64.zip

# Create packaged plugins
gzip -c /liblightstep_tracer_plugin.so > /linux-amd64-liblightstep_tracer_plugin.so.gz

# Create release
cd "${SRC_DIR}"
VERSION_TAG="`git describe --abbrev=0 --tags`"

RELEASE_TITLE="${VERSION_TAG/v/Release }" 
# No way to set title see https://github.com/tcnksm/ghr/issues/77

echo "/ghr -t <hidden> \
     -u $CIRCLE_PROJECT_USERNAME \
     -r $CIRCLE_PROJECT_REPONAME \
     -replace \
     "${VERSION_TAG}" \
     /linux-amd64-liblightstep_tracer_plugin.so.gz"
/ghr -t $GITHUB_TOKEN \
     -u $CIRCLE_PROJECT_USERNAME \
     -r $CIRCLE_PROJECT_REPONAME \
     -replace \
     "${VERSION_TAG}" \
     /linux-amd64-liblightstep_tracer_plugin.so.gz
