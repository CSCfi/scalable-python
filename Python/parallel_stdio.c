/*  Copyright (C) 2010       CSC - IT Center for Science Ltd.
*/

#include <mpi.h>
#include <stdio.h>
#include <sys/stat.h>

#define MASTER 0
#define MAX_FILES FOPEN_MAX // 5000 Maximum number of files to open in parallel

static int rank = MASTER; 
static int enabled = 0;
static int initialized = 0;

static FILE *parallel_fps[MAX_FILES];
static int current_fp = 0;
static FILE *fp_dev_null;

// Initialize wrapper stuff
void init_io_wrappers() 
{
  int i, mpi_initialized;
  MPI_Initialized(&mpi_initialized);
  if (mpi_initialized)
  {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (i=0; i < MAX_FILES; i++)
      parallel_fps[i] = -1;
    fp_dev_null = fopen("/dev/null", "rb");
    enabled = 1;
    initialized = 1;
  }
}

// switching wrapping on and off
void enable_io_wrappers()
{
  enabled = 1;
}

void disable_io_wrappers()
{
  enabled = 0;
}

// Utility function to check if the file pointer is "parallel"
int check_fp(FILE *fp)
{
  int i;
  for (i=current_fp-1; i >=0; i--)
    if ( fp == parallel_fps[i] )
      return i+1;

  return 0;
}


// File open, close, lock, etc.
FILE* __wrap_fopen(const char *filename, const char *modes)
{
  FILE *fp;
  int fp_is_null;
  if (! initialized)
    init_io_wrappers();
  // Wrap only in read mode
  if (modes[0] == 'r' && enabled) {
    if (rank == MASTER) {
#ifdef DEBUG_MPI
      fprintf(stderr, "[%d] opening file '%s'\n", rank, filename);
#endif
      fp = fopen(filename, modes);
      // NULL information is needed also in other ranks
      fp_is_null = ((fp == NULL) ? 1 : 0);
      MPI_Bcast(&fp_is_null, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    } else {
      MPI_Bcast(&fp_is_null, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
      if ( fp_is_null)
        fp = NULL;
      else
        fp = fp_dev_null;
    } 
    // Store the "parallel" file pointer
    if (fp != NULL) {
      parallel_fps[current_fp] = fp;
      current_fp++;
      if (current_fp == MAX_FILES) {
        printf("Too many open files\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
  } else {
    // Write mode, all processes can participate
#ifdef DEBUG_MPI
    fprintf(stderr, "[%d] opening file '%s'\n", rank, filename);
#endif
    fp = fopen(filename, modes);
  }
  return fp;
}

int  __wrap_fclose(FILE *fp)
{
  int x;
  int i = check_fp(fp);
  if ( i == current_fp && i > 0 )
    current_fp--;
  if ( ! i || (rank == MASTER) ) 
  {
#ifdef DEBUG_MPI
    fprintf(stderr, "[%d] closing file\n", rank);
#endif
    x = fclose(fp);
  }

  // TODO: Should one return the true return value of fclose?
  return 0;
}

void  __wrap_setbuf(FILE *fp, char *buf)
{
  if ( ! check_fp(fp) || (rank == MASTER) ) 
    setbuf(fp, buf);
}

int  __wrap_setvbuf(FILE *fp, char *buf, int type, size_t size)
{
  int x;
  if (check_fp(fp)) 
  {
    if (rank == MASTER) 
    {
      x = setvbuf(fp, buf, type, size);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
  }
  else
    x = setvbuf(fp, buf, type, size);
  return x;
}

int  __wrap_flockfile(FILE *fp)
{
  if ( ! check_fp(fp) || (rank == MASTER) ) 
    flockfile(fp);
  return 0;
}

int  __wrap_funlockfile(FILE *fp)
{
  if ( ! check_fp(fp) || (rank == MASTER) ) 
    funlockfile(fp);
  return 0;
}

int __wrap_ferror(FILE* fp)
{
  int x;
  if (check_fp(fp)) 
  {
    if (rank == MASTER) 
    {
      x = ferror(fp);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
  }
  else
    x = ferror(fp);
  return x;
}

int __wrap_feof(FILE* fp)
{
  int x;
  if (check_fp(fp)) 
  {
    if (rank == MASTER) 
    {
      x = feof(fp);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
  }
  else
    x = feof(fp);
  return x;
}

void  __wrap_clearerr(FILE *fp)
{
  if ( ! check_fp(fp) || (rank == MASTER) ) 
    clearerr(fp);
}

// File positioning etc.
int __wrap_fseek(FILE *fp, long offset, int origin)
{
  int x;
  if (check_fp(fp)) 
  {
    if (rank == MASTER) 
    {
      x = fseek(fp, offset, origin);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
  }
  else
    x = fseek(fp, offset, origin);
  return x;
}

void __wrap_rewind(FILE *fp)
{
  if (! check_fp(fp) || (rank == MASTER)) 
    rewind(fp);
}


int __wrap_ungetc(int c, FILE* fp)
{
  int x;
  if (enabled) 
    if (rank == MASTER) 
    {
      x =ungetc(c, fp);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
  else
    x =ungetc(c, fp);
  return x;
}

int __wrap_fflush(FILE *fp)
{
  int x;
  if (check_fp(fp)) 
  {
    if (rank == MASTER) 
    {
      x = fflush(fp);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
  }
  else
    x = fflush(fp);
  return x;
}

int __wrap_fgetpos ( FILE * fp, fpos_t * pos )
{
  int x;
  if (enabled) 
    if (rank == MASTER) 
    {
      x = fgetpos(fp, pos);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
  else
    x = fgetpos(fp, pos);
  return x;
}

int __wrap_fsetpos ( FILE * fp, const fpos_t * pos )
{
  int x;
  if (enabled) 
    if (rank == MASTER) 
    {
      x = fsetpos(fp, pos);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
  else
    x = fsetpos(fp, pos);
  return x;
}

long int __wrap_ftell ( FILE * fp )
{
  long x;
  if (enabled) 
    if (rank == MASTER) 
    {
      x = ftell(fp);
      MPI_Bcast(&x, 1, MPI_LONG, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(&x, 1, MPI_LONG, MASTER, MPI_COMM_WORLD); 
  else
    x = ftell(fp);
  return x;
}

// Read functions
int __wrap_getc(FILE *fp)
{
  int x;
  if (enabled) 
    if (rank == MASTER )
    {
      x = getc(fp);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    }
    else
    {
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    }
  else
    x = getc(fp);
  return x;
}

int __wrap_getc_unlocked(FILE *fp)
{
  int x;
  if (enabled) 
    if (rank == MASTER )
    {
      x = getc_unlocked(fp);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    }
    else
    {
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    }
  else
    x = getc_unlocked(fp);
  return x;
}

int __wrap_fread(void *ptr, size_t size, size_t n, FILE* fp)
{
  // Is it OK to use just int for the size of data read?
  int x;
  if (enabled) 
    if (rank == MASTER) 
    {
      x = fread(ptr, size, n, fp);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
      MPI_Bcast(ptr, x*size, MPI_BYTE, MASTER, MPI_COMM_WORLD); 
    }
    else
    {
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
      MPI_Bcast(ptr, x*size, MPI_BYTE, MASTER, MPI_COMM_WORLD); 
    }
  else
    x = fread(ptr, size, n, fp);
  return x;
}

char *__wrap_fgets(char *str, int num, FILE* fp)
{
  char* s;
  int s_is_null=0;
  if (enabled) 
    if (rank == MASTER) 
    {
      s = fgets(str, num, fp);
      if (s==NULL)
      {
        s_is_null = 1;
        MPI_Bcast(&s_is_null, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
      }
      else
      {
        MPI_Bcast(&s_is_null, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
        MPI_Bcast(s, num, MPI_BYTE, MASTER, MPI_COMM_WORLD); 
      }
    }
    else
    {
      MPI_Bcast(&s_is_null, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
      if (s_is_null == 1)
        s = NULL;
      else
        MPI_Bcast(s, num, MPI_BYTE, MASTER, MPI_COMM_WORLD); 
    }    
  else   
    s = fgets(str, num, fp);
  return s;
}

int __wrap_fgetc ( FILE * fp )
{
  int x;
  if (enabled) 
    if (rank == MASTER )
    {
      x = fgetc(fp);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    }
    else
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  else
    x = fgetc(fp);
  return x;
}

int __wrap_fstat(int fildes, struct stat *buf)
{
  int size = sizeof(struct stat);
  if (enabled) 
    if (rank == MASTER) 
    {
      fstat(fildes, buf);
      MPI_Bcast(buf, size, MPI_BYTE, MASTER, MPI_COMM_WORLD); 
    }
    else
      MPI_Bcast(buf, size, MPI_BYTE, MASTER, MPI_COMM_WORLD); 
  else
    fstat(fildes, buf);

  // TODO should one return the true return value?
  return 0;
}

int __wrap_stat(const char *path, struct stat *buf)
{
  int size = sizeof(struct stat);
  int x;
  if (enabled)
    if (rank == MASTER)
    {
      x = stat(path, buf);
      MPI_Bcast(buf, size, MPI_BYTE, MASTER, MPI_COMM_WORLD);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    }
    else
    {
      MPI_Bcast(buf, size, MPI_BYTE, MASTER, MPI_COMM_WORLD);
      MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    }
  else
    x = stat(path, buf);

  return x;
}


/* fileno is not actually needed
   int __wrap_fileno( FILE *fp )
   {
   int x;
   if (check_fp(fp)) 
   {
   if (rank == MASTER) 
   {
   x = fileno(fp);
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
   }
   else
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
   }
   else
   x = fileno(fp);
   return x;
   }
   */


// Write functions
/*
   int __wrap_fputc ( int character, FILE * fp )
   {
   int x;
   if (rank == MASTER) 
   {
   x =  fputc(character, fp);
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
   }
   else
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 

   return x;
   }

   int __wrap_fputs ( const char * str, FILE * fp )
   {
   int x;
   if (rank == MASTER) 
   {
   x = fputs(str, fp);
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
   }
   else
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 

   return x;
   }

   int __wrap__IO_putc ( int character, FILE * fp )
   {
   int x;
   if (rank == MASTER) 
   {
   x = _IO_putc(character, fp);
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
   }
   else
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
   return x;
   }

   size_t __wrap_fwrite ( const void * ptr, size_t size, size_t count, FILE * fp )
   {
   int x;
   if (rank == MASTER) 
   {
   x = fwrite(ptr, size, count, fp);
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
   }
   else
   MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD); 
   return x;
   }
   */

/* vim: set et tw=80 ts=2 sw=2: */
