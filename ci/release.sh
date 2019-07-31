#!/bin/bash

set -e
apt-get update 
apt-get install --no-install-recommends --no-install-suggests -y \
                wget \
                unzip \
                python python-pip
pip install twine

# Install ghr
cd /
wget https://github.com/tcnksm/ghr/releases/download/v0.5.4/ghr_v0.5.4_linux_amd64.zip
unzip ghr_v0.5.4_linux_amd64.zip

# Create packaged plugins
gzip -c /plugin/liblightstep_tracer_plugin.so > /linux-amd64-liblightstep_tracer_plugin.so.gz

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

# Upload the wheel to pypi
echo -e "[pypi]" >> ~/.pypirc
echo -e "username = $PYPI_USER" >> ~/.pypirc
echo -e "password = $PYPI_PASSWORD" >> ~/.pypirc
twine upload /plugin/*.whl
