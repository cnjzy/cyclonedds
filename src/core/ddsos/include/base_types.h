#ifndef _BASE_TYPES_H
#define _BASE_TYPES_H

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long ULONG;
typedef unsigned long long UINT64;

typedef signed char INT8;
typedef signed short INT16;
typedef signed int INT32;
typedef signed long long INT64;

typedef float SINGLE;
typedef double DOUBLE;
#ifndef _WIN32
typedef unsigned int BOOL;
typedef unsigned int boolean;
#endif

typedef int STATUS;
/*typedef void VOID;*/
typedef void* OBJ;
typedef void* VOID_PTR;
typedef unsigned char UCHAR;
typedef char CHAR;

typedef enum RET_CODE_T {
	RET_INVALID_DATA = -5,
	RET_BUF_OVERFLOW = -4,
	RET_TIMED_OUT = -3,
	RET_INVALID_PARAM = -2,
	RET_ERR = -1,
	RET_NO_ERR = 0
} RET_CODE_T;

typedef struct STRING_T {
	CHAR* data;
	UINT32 len;
} STRING_T;


#endif	/* _TYPES_H*/
