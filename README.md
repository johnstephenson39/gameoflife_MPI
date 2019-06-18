# Implementation of the Game of Life using MPI
1. Project title
    * Multithreaded version of the "Game of Life" using MPI
2. Project Description
    * An implementation of a multithreaded version of the "Game of Life" program using MPI.
3. Author
    * John Stephenson
4. Acknowledgment
    * https://hpcdocs.asc.edu/content/supercomputer-hardware
    * An Introduction to Parallel Programming by Peter Pacheco
5. Getting Started
    * Prerequisites/dependencies
        * #include cstdlib, cstdio, iostream, mpi.h
    * Instructions for building the software
        * mpic++ -O3 gol.cpp -o gol
        * mpirun -np (num procs) ./gol (N) (max generations)
6. Contact Information
    * johnds39@uab.edu
