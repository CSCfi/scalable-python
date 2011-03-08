/* Minimal main program -- everything is loaded from the library */

#include "Python.h"
#include <mpi.h>

#ifdef __FreeBSD__
#include <floatingpoint.h>
#endif

int
main(int argc, char **argv)
{
	/* 754 requires that FP exceptions run in "no stop" mode by default,
	 * and until C vendors implement C99's ways to control FP exceptions,
	 * Python requires non-stop mode.  Alas, some platforms enable FP
	 * exceptions by default.  Here we disable them.
	 */
   int mystatus;
   MPI_Init(&argc, &argv);
#ifdef __FreeBSD__
	fp_except_t m;

	m = fpgetmask();
	fpsetmask(m & ~FP_X_OFL);
#endif
	mystatus = Py_Main(argc, argv);
     MPI_Finalize();
       return mystatus;
}
