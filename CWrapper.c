
/*
	cWrapper.c

	Set of functions wrappers to common C library functions.

	Miguel Leitao, 2025
*/
 
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <dlfcn.h>

void *libcHandler = NULL;



int StrNumChar(char *s, char c) {
  // Counts occurences of character 'c' in string 's'.
  int n = 0;
  while (*s) {
    if (*s==c) n++;
    s++;
  }
  return n;
}


int fprintf_parse(FILE *fout, int nargs, char *format, va_list ap) {
  int i;
  char *ini = format;
  for( i=0 ; i<nargs ; i++ ) {
    char *end = strchr(ini,'%')+1;
    while (*end=='.' || (*end>='0' && *end<='9') ) end++;
    char *new = strndup(ini,end-ini+1);
    switch (*end) {
      case 'c':
      case 'd': {
        int d = va_arg(ap, int);
        fprintf(fout, new, d);
      } break;
      case 'f': {
        double d = va_arg(ap, double);
        fprintf(fout, new, d);
      } break;
      case 's': {
        char *d = va_arg(ap, char*);
        fprintf(fout, new, d);
      } break;
      default:
        fprintf(stderr,"\ninvalid fomat\n");
    }
    ini = end+1;
  }
  fputs(ini,fout);
  return nargs;
}

int FPrintF(FILE *fout, char *format, ... ) {
  int nargs;
  nargs = StrNumChar(format, '%');
  //printf("nargs=%d\n", nargs);
  va_list ap;
  va_start(ap, format);
    fprintf_parse(fout, nargs, format, ap);
  va_end(ap);
  return nargs;
}

int PrintF(char *format, ... ) {
  int nargs;
  nargs = StrNumChar(format, '%');
  //printf("nargs=%d\n", nargs);
  va_list ap;
  va_start(ap, format);
    fprintf_parse(stdout, nargs, format, ap);
  va_end(ap);
  return nargs;
}

void *loadLibC() {
  if ( libcHandler ) return libcHandler;
  libcHandler = dlopen("libc.so", RTLD_LAZY);
  if ( libcHandler ) return libcHandler;
  FPrintF(stderr, "Error: Could not load libc\n");
  return NULL;
}

int atoi(char *s) {
    if ( ! loadLibC() ) return 0;
    dlerror();
    int (*libcAtoi)(char *) = dlsym(libcHandler, "atoi");
    char *error = dlerror();
    if ( error != NULL ) {
        FPrintF(stderr, "Error: %s\n", error);
        //exit(1);
        return 0;
    }
    return (*libcAtoi)(s);
}

#ifdef TEST_APP

int main() {
  FPrintF(stdout,"%d+%d=%d, %f+%f=%f, str:'%s'\n",6,2,8,4.5,5.3,9.8,"all done");
  FPrintF(stdout,"35=%d\n", atoi("35"));
  return 0;
}

#endif
