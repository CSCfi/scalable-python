#ifdef ENABLE_MPI
#define fopen  __wrap_fopen
#define fclose  __wrap_fclose
#define setbuf  __wrap_setbuf
#define setvbuf __wrap_setvbuf
#define flockfile __wrap_flockfile
#define funlockfile __wrap_funlockfile
#define ferror __wrap_ferror
#define feof __wrap_feof
#define clearerr __wrap_clearerr
#define fseek __wrap_fseek
#define rewind __wrap_rewind
#define ungetc __wrap_ungetc
#define fflush __wrap_fflush
#define fgetpos __wrap_fgetpos
#define fsetpos __wrap_fsetpos
#define ftell  __wrap_ftell
#define getc __wrap_getc
#define getc_unlocked __wrap_getc_unlocked
#define fread __wrap_fread
#define fgets __wrap_fgets
#define fgetc __wrap_fgetc
#define fstat __wrap_fstat
#define stat(path, buf) __wrap_stat(path, buf)

FILE* __wrap_fopen(const char *filename, const char *modes);
int  __wrap_fclose(FILE *fp);
void  __wrap_setbuf(FILE *fp, char *buf);
int  __wrap_setvbuf(FILE *fp, char *buf, int type, size_t size);
int  __wrap_flockfile(FILE *fp);
int  __wrap_funlockfile(FILE *fp);
int __wrap_ferror(FILE* fp);
int __wrap_feof(FILE* fp);
void  __wrap_clearerr(FILE *fp);
int __wrap_fseek(FILE *fp, long offset, int origin);
void __wrap_rewind(FILE *fp);
int __wrap_ungetc(int c, FILE* fp);
int __wrap_fflush(FILE *fp);
int __wrap_fgetpos ( FILE * fp, fpos_t * pos );
int __wrap_fsetpos ( FILE * fp, const fpos_t * pos );
long int __wrap_ftell ( FILE * fp );
int __wrap_getc(FILE *fp);
int __wrap_getc_unlocked(FILE *fp);
int __wrap_fread(void *ptr, size_t size, size_t n, FILE* fp);
char *__wrap_fgets(char *str, int num, FILE* fp);
int __wrap_fgetc ( FILE * fp );
int __wrap_fstat(int fildes, struct stat *buf);
int __wrap_stat(const char *path, struct stat *buf);

void init_io_wrappers(void);
void finalize_io_wrappers(void);
#endif

void enable_io_wrappers(void);
void disable_io_wrappers(void);
