#! /bin/bash

# Ensure docker
if [ ! -f /.dockerenv ]; then
    echo "This file should be run inside a docker container! See README.md"
    exit 1
fi

# Prepare EDK2 base
pushd edk2
make -C BaseTools
. edksetup.sh
popd

# Copy SmmMap-related files and changes
cp -r SmmMap/ edk2/
cp -r edk2-overrides/* edk2

# Build EDK2 UEFI firmware image with Plouton
pushd edk2
. edksetup.sh

# We unfortunately need to remap the paths to host FS
# So after building run: "sed -i "s|/root/edk2/|$(pwd)/edk2/|g" compile_commands.json"
bear -o ../compile_commands.json build -DSMM_REQUIRE
#build -DSMM_REQUIRE
popd

# Clean-up SmmMap-related changes
# --> Not everything! edk2-overrides need to be reverted using git:
#                     run "git restore ." (inside edk2 directory)
#                     to get completely clean state 
rm -rf edk2/Plouton
