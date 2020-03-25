#ifndef WS_CANVAS_API_H
#define WS_CANVAS_API_H

#ifdef WS_CANVAS_BUILT_AS_STATIC
#  define WS_CANVAS_API
#  define WS_CANVAS_NO_EXPORT
#else
#  ifndef WS_CANVAS_API
#    ifdef WS_CANVAS_EXPORTS
/* We are building this library */
#      define WS_CANVAS_API __declspec(dllexport)
#    else
/* We are using this library */
#      define WS_CANVAS_API __declspec(dllimport)
#    endif
#  endif

#  ifndef WS_CANVAS_NO_EXPORT
#    define WS_CANVAS_NO_EXPORT
#  endif
#endif

#ifndef WS_CANVAS_DEPRECATED
#  define WS_CANVAS_DEPRECATED __declspec(deprecated)
#endif

#ifndef WS_CANVAS_DEPRECATED_EXPORT
#  define WS_CANVAS_DEPRECATED_EXPORT WS_CANVAS_API WS_CANVAS_DEPRECATED
#endif

#ifndef WS_CANVAS_DEPRECATED_NO_EXPORT
#  define WS_CANVAS_DEPRECATED_NO_EXPORT WS_CANVAS_NO_EXPORT WS_CANVAS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef WS_CANVAS_NO_DEPRECATED
#    define WS_CANVAS_NO_DEPRECATED
#  endif
#endif

#endif /* WS_CANVAS_API_H */
