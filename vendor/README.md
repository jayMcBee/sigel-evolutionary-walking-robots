# vendor

The third-party code SIGEL is built on, and our fixes to it.

`pvm3.4.6.tgz` is the upstream PVM archive. `supportingLibs.tar.gz` holds the
four libraries the build uses, `cv97`, `dynamechs`, `fparser` and
`newmat09`, unchanged from SIGEL's original supportingLibs archive. Both are
tracked here so the project builds without downloading anything. From the
repo root:

    mkdir -p downloads
    tar xzf vendor/supportingLibs.tar.gz -C downloads
    tar xzf vendor/pvm3.4.6.tgz -C downloads/supportingLibs \
        --strip-components=1 ./pvm3

**Never extract over an existing tree**: `tar` overwrites but never deletes, so
our patch stamp survives, and the build keeps objects whose sources are
gone. Remove the tree first. The `Makefile` header explains
the traps in full.

`patches/` holds the 12 fixes `make` applies to
`downloads/supportingLibs/`, so that it builds and runs with a current
toolchain.
