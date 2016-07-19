/* E: error
 * W: warning
 * D: debug
 * I: info
 */

#include <sys/time.h>  //timeval
#include <stdio.h>  //fprintf, stderr
#include <stdarg.h> //vfprintf
#include <stdint.h> //for uintXX_t


/*
#define LOG(_fmt, ...)						\
    if () {							\
	struct timeval _t0;					\
	gettimeofday(&_t0, NULL);				\
	fprintf(stderr, "%03d.%06d %-10.10s [%d] " _fmt "\n",	\
	    (int)(_t0.tv_sec % 1000), (int)_t0.tv_usec,		\
	    __FUNCTION__, __LINE__, ##__VA_ARGS__);		\
    } while (0)
*/  
    
struct _header_t{ 
	uint32_t magic_num; 
	uint32_t dictionary_size; 
	uint32_t symbol_size; 
	char filename[256];
} header_t;    
    
static inline void LOG(const char* format, ...){
	va_list argptr;
	va_start(argptr, format);
	fprintf(stderr, "funzione: %s, file&linea: %s, %d, istante: %s\n", __func__, __FILE__, __LINE__, __TIME__);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
}
    
    
    /*
    void Error(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}
    */

    
