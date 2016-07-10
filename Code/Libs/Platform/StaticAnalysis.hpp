#pragma once

/// Static code analysis enabled
#ifndef AX_STATIC_ANALYSIS
# if defined( _MSC_VER ) && defined( _PREFAST_ )
#  define AX_STATIC_ANALYSIS 1
# else
#  define AX_STATIC_ANALYSIS 0
# endif
#endif

#if AX_STATIC_ANALYSIS && defined( _MSC_VER )
# include <sal.h>
#endif

/// Declare that the next instance of a warning should be ignored
#ifndef AX_STATIC_SUPPRESS
# ifdef _MSC_VER
#  define AX_STATIC_SUPPRESS(code) __pragma( warning( suppress: code ) )
# else
#  define AX_STATIC_SUPPRESS(code)
# endif
#endif

/// Tell the analyzer to assume a certain expression to be true
#ifndef AX_STATIC_ASSUME
# ifdef _MSC_VER
#  define AX_STATIC_ASSUME(expr) __analysis_assume( !!( expr ) )
# else
#  define AX_STATIC_ASSUME(expr)
# endif
#endif

/// "In" annotation
#ifndef AX_IN
# if AX_STATIC_ANALYSIS && defined( _MSC_VER )
#  define AX_IN __in
# else
#  define AX_IN
# endif
#endif

/// "Out" annotation
#ifndef AX_OUT
# if AX_STATIC_ANALYSIS && defined( _MSC_VER )
#  define AX_OUT __out
# else
#  define AX_OUT
# endif
#endif

/// Declare that the given parameter will only be read
#ifndef AX_READ_ONLY
# if AX_STATIC_ANALYSIS && defined( _MSC_VER )
#  define AX_READ_ONLY __readonly //[Pre(Access=Read)]
# else
#  define AX_READ_ONLY
# endif
#endif

/// Declare that the given parameter will only be written to
#ifndef AX_WRITE_ONLY
# if AX_STATIC_ANALYSIS && defined( _MSC_VER )
#  define AX_WRITE_ONLY _Pre1_impl_(__writeaccess_impl) //[Pre(Access=Write)]
# else
#  define AX_WRITE_ONLY
# endif
#endif

/// Declare that the given pointer must be non-NULL (and valid)
#ifndef AX_NOT_NULL
# if AX_STATIC_ANALYSIS && defined( _MSC_VER )
#  define AX_NOT_NULL __notnull //[Pre(Null=No,Valid=Yes)]
# else
#  define AX_NOT_NULL
# endif
#endif

/// Declare that the return value of a function is always not null
#ifndef AX_RETURN_NOT_NULL
# if AX_STATIC_ANALYSIS && defined( _MSC_VER )
#  define AX_RETURN_NOT_NULL _Ret_notnull_
# else
#  define AX_RETURN_NOT_NULL
# endif
#endif

/// Declare that the return value of a function is always null
#ifndef AX_RETURN_NULL
# if AX_STATIC_ANALYSIS && defined( _MSC_VER )
#  define AX_RETURN_NULL _Ret_null_
# else
#  define AX_RETURN_NULL
# endif
#endif

/// Declare that a caller to a function must check the return value from that function
#ifndef AX_CHECK_RETURN
# if AX_STATIC_ANALYSIS && defined( _MSC_VER )
#  define AX_CHECK_RETURN _Check_return_
# else
#  define AX_CHECK_RETURN
# endif
#endif
