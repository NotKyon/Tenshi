#pragma once

#include "../Platform/Platform.hpp"

/// \def AX_VECTOR_ALIGNMENT
/// \brief The required alignment for vectors.
///
/// \def AX_VECALIGN
/// \brief Same as <tt>AX_ALIGN( AX_VECTOR_ALIGNMENT )</tt>.

// include the appropriate intrinsics headers
#if AX_INTRIN & AX_INTRIN_SSE
# include <mmintrin.h>
# include <emmintrin.h>
# include <xmmintrin.h>
# ifndef AX_VECTOR_ALIGNMENT
#  define AX_VECTOR_ALIGNMENT 16
# endif
#elif AX_INTRIN == 0
# ifndef AX_VECTOR_ALIGNMENT
#  define AX_VECTOR_ALIGNMENT 1
# endif
#else
# error Unhandled intrinsics system
#endif

// figure out vector alignment
#ifndef AX_VECALIGN
# if AX_VECTOR_ALIGNMENT > 1
#  define AX_VECALIGN AX_ALIGN( AX_VECTOR_ALIGNMENT )
# else
#  define AX_VECALIGN
# endif
#endif
