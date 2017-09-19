# Scalable Python 1.x

> based on Python 2.7.13
>
> &copy; 2011-2017
>  [CSC - IT Center for Science Ltd.](http://github.com/CSCfi)   
> authors:
>  [Jussi Enkovaara](http://github.com/jussienko),
>  [Martti Louhivuori](http://github.com/mlouhivu)

Special Python interpreter intended for massively parallel HPC systems.

When using Python for parallel applications, standard Python puts
lots of pressure on the filesystem, as during import statements
several processes try to access several small files. With thousands
of processes startup time of Python application can be more than
30 minutes.

This special interpreter performs the I/O operations used by "import"
statements only by single process, and uses MPI for transmitting data to
all other process.

## Short build instructions

Use `--enable-mpi` flag for configure, e.g.

    ./configure --enable-mpi

Build and install standard Python interpreter

    make
    make install

Build and install the special Python interpreter

    make mpi
    make install-mpi

## Short usage instructions

The procedure described above creates two executables:

- **python** is the standard Python interpreter, which can be used for normal
  serial applications and e.g. for software installations.
- **python_mpi** is the special Python interpreter which has to be started with
  `mpirun` (or equivalent MPI launch command), e.g.

```
mpirun -np 16384 python_mpi my_application.pyi
```

## Disabling I/O wrappers

It is possible to disable the special I/O wrappers on-the-fly and revert to
the standard POSIX calls by using the built-in function `wrapoff()` in a
Python script. For example:

    wrapoff()
    from numpy import array

Similarly, they can be turned back on with the built-in function `wrapon()`.

## Limitations

All the process have to perform the same "import" statements, e.g. code like

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

## Other build considerations

By default, the MPI related parts in special interpreter are build by `mpicc`.
Alternate MPI compiler can be specified with variable MPICC.
