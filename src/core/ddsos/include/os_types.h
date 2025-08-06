#ifndef OS_TYPES_H
#define OS_TYPES_H

#if defined(_WIN32)
#define	OS_API __declspec(dllexport)
#else
#define OS_API 
#endif

#endif /*/ !OS_TYPES_H*/
