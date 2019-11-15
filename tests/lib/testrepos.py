#!/usr/bin/python3

import argparse
import shutil
import sys
import tempfile
import subprocess

parser = argparse.ArgumentParser(description='Set up and serve a test repository.')
parser.add_argument("sourceDir")

args = parser.parse_args()

rpmbuildBin = shutil.which("rpmbuild")
if rpmbuildBin == None:
    print("The rpmbuild binary was not found in PATH")
    sys.exit(1)

createRepo = shutil.which("createrepo")
if createRepo == None:
    print("The createrepo binary was not found in PATH")
    sys.exit(1)

#create our rpmbuild directory
builddir = tempfile.mkdtemp()
if builddir == None:
    print("Unable to create temporary directory")
    sys.exit(1)

rpmbuildCmd = "{} -D '%topdir {}' -D '%_builddir %{{topdir}}/build' -D '%_rpmdir %{{topdir}}/rpms' -D '%_srcrpmdir %{{topdir}}/rpms' -D '%_buildrootdir %{{topdir}}/buildroot' -bb *.spec".format(
    rpmbuildBin,
    builddir
)

print("Building the example RPMs")

result = subprocess.call([
    "bash",
    "-c",
    rpmbuildCmd
], cwd=args.sourceDir)

if result != 0:
    sys.exit(1)

print("Building the spec files worked, going to create the repository\n")
result = subprocess.call([
    createRepo,
    "{}/rpms".format(builddir)
])

if result != 0:
    sys.exit(1)






