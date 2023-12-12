# Bachelors Thesis - Solving Sudoku using DLX and MPI
The source code and pdf of my finished thesis that I wrote in Spring 2023. The main objective of my project was to explore the parallelisation opportunities of Donald Knuth's Algorithm DLX/X using the Message Passing Interface (MPI) library in C. I implemented two different versions, one using the normal Message Passing paradigm of MPI and one using the One-Sided Communication paradigm of MPI.

Funnily enough, when I did some test runs this autumn for fun after the CS insitution had upgraded the servers to Debian 12, the scalability of both Message Passing and One-Sided Communication improved a fair bit with less of a drop-off with increasing core count. 
