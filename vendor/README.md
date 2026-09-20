# vendor

The third-party code SIGEL is built on, and our fixes to it.

`supportingLibs.tar.gz` and `pvm3.4.6.tgz` are the upstream archives, tracked
here so the project builds without downloading anything. From the repo root:

    tar xzf vendor/supportingLibs.tar.gz -C x/supportingLibs
    rm -rf x/supportingLibs/supportingLibs/pvm3
    tar xzf vendor/pvm3.4.6.tgz -C x/supportingLibs/supportingLibs \
        --strip-components=1 ./pvm3

**Never extract over an existing tree**: `tar` overwrites but never deletes, so
the old PVM comes back, our patch stamp survives, and the build keeps objects
whose sources are gone. Remove the tree first. The `Makefile` header explains
the traps in full.

`patches/` holds the 12 fixes `make` applies to
`x/supportingLibs/supportingLibs/`, so that it builds and runs with a current
toolchain.
