# Scalable Python 1.x

> based on Python 2.7.13 (see original [README](README.orig))
>
> &copy; 2011-2017
>  [CSC - IT Center for Science Ltd.](http://github.com/CSCfi)   
> authors:
>  [Jussi Enkovaara](http://github.com/jussienko) &
>  [Martti Louhivuori](http://github.com/mlouhivu)

***Special Python interpreter intended for massively parallel HPC systems.***

When using Python for parallel applications, standard Python puts a lot of
pressure on the filesystem with multiple processes accessing several small
files during import statements. With thousands of processes the startup time
of a Python application can be more than 30 minutes.

**Scalable Python** performs the I/O operations used e.g. by import statements in
a single process and uses MPI to transmit data to/from all other processes.
Please see the [inherent limitation](#limitations) of this approach.

## Short build instructions

Use `--enable-mpi` option for configure, e.g.

    ./configure --enable-mpi

Build and install normal Python interpreter (`python`) as well as standard
modules etc.

    make
    make install

Build and install the special Python interpreter (`python_mpi`)

    make mpi
    make install-mpi

By default, the MPI related parts are build using `mpicc`, but an alternative
MPI compiler can be specified using the environment variable MPICC (e.g.
`export MPICC=cc`).

## Short usage instructions

Installation creates two executables:

- **python** is the standard Python interpreter, which can be used for normal
  serial applications and e.g. for software installations.
- **python_mpi** is the special Python interpreter that wraps I/O calls and
  uses MPI for communication, and thus has to be started with `mpirun` (or
  an equivalent MPI launch command), e.g.

```
mpirun -np 16384 python_mpi my_application.py
```

## Disabling I/O wrappers

It is possible to disable the special I/O wrappers on-the-fly and revert to
the standard POSIX calls by using the built-in function `wrapoff()` in a
Python script. For example:

    wrapoff()
    from numpy import array

Similarly, they can be turned back on with the built-in function `wrapon()`.

## Limitations

All MPI processes have to perform the same I/O operations (e.g. import
statements). For example, a code like this

    if rank == 0:
        import mymodule

is NOT allowed. If you need to do it, please use the `wrapoff()` and
`wrapon()` functions to disable the special I/O wrappers temporarily.

If you experience an unexpected deadlock while using Scalable Python, it is
most likely due to triggering this limitation. Unfortunately, even some
standard modules use in-function import statements, so the root cause may turn
out to be an innocent looking function call within an if statement. If needed,
you can try to turn off the special I/O wrappers e.g. after the initial module
imports.
