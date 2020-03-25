#ifndef WS_API_H
#define WS_API_H

#ifdef WS_BUILT_AS_STATIC
#  define WS_API
#  define WS_NO_EXPORT
#else
#  ifndef WS_API
#    ifdef WS_EXPORTS
/* We are building this library */
#      define WS_API __declspec(dllexport)
#    else
/* We are using this library */
#      define WS_API __declspec(dllimport)
#    endif
#  endif

#  ifndef WS_NO_EXPORT
#    define WS_NO_EXPORT
#  endif
#endif

#ifndef WS_DEPRECATED
#  define WS_DEPRECATED __declspec(deprecated)
#endif

#ifndef WS_DEPRECATED_EXPORT
#  define WS_DEPRECATED_EXPORT WS_API WS_DEPRECATED
#endif

#ifndef WS_DEPRECATED_NO_EXPORT
#  define WS_DEPRECATED_NO_EXPORT WS_NO_EXPORT WS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef WS_NO_DEPRECATED
#    define WS_NO_DEPRECATED
#  endif
#endif

#endif /* WS_API_H */
