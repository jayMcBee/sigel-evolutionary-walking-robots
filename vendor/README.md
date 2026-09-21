# vendor

The third-party code SIGEL is built on, and our fixes to it.

`pvm3.4.6.tgz` is the upstream PVM archive. `supportingLibs.tar.gz` is
SourceForge's archive cut down to the five libraries the build uses: `cv97`,
`dynamechs`, `Dynamo`, `fparser` and `newmat09`, unchanged. The full archive,
with Qt 2.3, `qhull`, `SOLID` and PVM 3.4.3, is on SourceForge and in git
history. Both are tracked here so the project builds without downloading
anything. From the repo root:

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
