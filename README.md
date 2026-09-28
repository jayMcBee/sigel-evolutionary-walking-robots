# SIGEL

An independent fork of the SIGEL project that modernizes the code in an attempt to bring it up to current standards.
It is not made, maintained or endorsed by SIGEL's original authors.

SIGEL evolves control programs for walking robots by genetic programming and
tests each program in a physics simulator.

Original project, by project group PG 368 at the University of Dortmund:
https://sigel.sourceforge.net/seiten/einleitung_en.html  
PG 368 built SIGEL 1.0, finished in August 2001: the GP system, the robot description language, the DynaMechs simulator, the OpenGL visualisation and the PVM master/slave split ([final report](https://sigel.sourceforge.net/download/berichte/endbericht.pdf), in German).  
The MetaGP system came later, in versions 1.1–1.3 (2002–2003), and implements the fitness meta-models that Jens Ziegler and Wolfgang Banzhaf published with SIGEL results ([CLAWAR 2003](http://www.cs.mun.ca/~banzhaf/papers/ZieglerBanzhaf.pdf)); as shipped, it cannot start on Linux.  
(Running the 1.3 binaries on today's Linux: [Quick Guide](verification-against-sigel-1.3/README.txt))

Third-party libraries: see [LIBRARIES.md](LIBRARIES.md).

License: GNU GPL, as the original.
