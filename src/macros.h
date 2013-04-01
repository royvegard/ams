#ifndef MACROS_H
#define MACROS_H

#if __SSE_MATH__ && __i386__
	#define FATTR_FLOAT_ARG __attribute__((sseregparm))
#else
	#define FATTR_FLOAT_ARG
#endif


#endif
