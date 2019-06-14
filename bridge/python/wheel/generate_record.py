import hashlib
import base64
import sys
import os

root = sys.argv[1]

for subdir, _, files in os.walk(root):
    for filename in files:
        path = os.path.join(subdir, filename)
        relative_path = path[len(root)+1:]
        with open(path, 'r') as f:
            data = f.read()
            num_bytes = len(data)
            sha256 = hashlib.sha256(data).digest()
        sha256 = base64.urlsafe_b64encode(sha256).rstrip('=')
        print "%s,%s,%d" % (relative_path, sha256, num_bytes)
