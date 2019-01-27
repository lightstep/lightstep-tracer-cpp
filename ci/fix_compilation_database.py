#!/usr/bin/python

# Taken from https://github.com/envoyproxy/envoy/blob/964dd1de5d4d4cb388a0e6dd25dbff1f7ff56cea/tools/gen_compilation_database.py with modification.

import json
import sys
import os

def isHeader(filename):
  for ext in (".h", ".hh", ".hpp", ".hxx"):
    if filename.endswith(ext):
      return True
  return False

def isCompileTarget(target):
  filename = target["file"]

  if filename.startswith("3rd_party"):
      return False

  if filename.startswith("bazel-out/"):
    return False

  if filename.startswith("external/"):
    return False

  return True

def modifyCompileCommand(target):
  cxx, options = target["command"].split(" ", 1)

  # Workaround for bazel added C++11 options, those doesn't affect build itself but
  # clang-tidy will misinterpret them.
  options = options.replace("-std=c++0x ", "")
  options = options.replace("-std=c++11 ", "")

  if isHeader(target["file"]):
    options += " -Wno-pragma-once-outside-header -Wno-unused-const-variable"
    options += " -Wno-unused-function"
    options += " -x c++-header"

  target["command"] = " ".join(["clang++", options])
  return target

with open("compile_commands.json", "r") as db_file:
    db = json.load(db_file)

db = [modifyCompileCommand(target) for target in db if isCompileTarget(target)]
os.remove("compile_commands.json")

with open("compile_commands.json", "w") as db_file:
   json.dump(db, db_file, indent=2)
