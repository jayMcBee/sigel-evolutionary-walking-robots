# SIGEL

An independent fork of the SIGEL project that modernizes the code in an attempt to bring it up to current standards.
It is not made, maintained or endorsed by SIGEL's original authors.

SIGEL evolves control programs for walking robots by genetic programming and
tests each program in a physics simulator.

## Origin

- **SIGEL 1.0 (2001):** written by project group PG 368 at the University of
  Dortmund, including the GP system, the robot language, the simulator and the
  visualisation
  ([project page](https://sigel.sourceforge.net/seiten/einleitung_en.html),
  [final report](https://sigel.sourceforge.net/download/berichte/endbericht.pdf), German).
- **MetaGP (1.1–1.3):** implements the fitness meta-models from Jens Ziegler
  and Wolfgang Banzhaf's research
  ([CLAWAR 2003](http://www.cs.mun.ca/~banzhaf/papers/ZieglerBanzhaf.pdf));
  it cannot currently start on Linux.
- The original 1.3 binaries still run on today's Linux: see the
  [Quick Guide](verification-against-sigel-1.3/README.txt).

Third-party libraries: see [LIBRARIES.md](LIBRARIES.md).

License: GNU GPL, as the original.
