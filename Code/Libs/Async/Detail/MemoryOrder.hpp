#pragma once

#if defined( _MSC_VER )
# include <intrin.h>
# pragma intrinsic( _ReadWriteBarrier )
# pragma intrinsic( _mm_mfence )
#elif defined( __GNUC__ ) || defined( __clang__ )
#else
# include <atomic>
#endif

#ifndef AX_COMPILER_FENCE
# if defined( _MSC_VER )
#  define AX_COMPILER_FENCE()		_ReadWriteBarrier()
# elif defined( __GNUC__ ) || defined( __clang__ )
#  define AX_COMPILER_FENCE()		__sync_synchronize() //__asm__ __volatile__( "" : : : "memory" )
# endif
#endif

#ifndef AX_PLATFORM_FENCE
# if defined( _MSC_VER )
#  define AX_PLATFORM_FENCE()		_mm_mfence()
# elif defined( __GNUC__ ) || defined( __clang__ )
#  if ( AX_PLATFORM & AX_PLATFORM_INTEL )
#   define AX_PLATFORM_FENCE()		__asm__ __volatile__( "mfence" : : : "memory" )
#  endif
# endif
#endif

#ifndef AX_MEMORY_BARRIER
# if defined( AX_PLATFORM_FENCE )
#  if defined( AX_COMPILER_FENCE )
#   define AX_MEMORY_BARRIER()		AX_COMPILER_FENCE(); AX_PLATFORM_FENCE()
#  else
#   define AX_MEMORY_BARRIER()		AX_PLATFORM_FENCE()
#  endif
# else
#  define AX_MEMORY_BARRIER()		std::atomic_thread_fence( std::memory_order_seq_cst )
# endif
#endif

namespace Ax { namespace Async {

	struct SMemorySemantic
	{
	};

	enum class EMemorySemantic
	{
		Acquire,
		Release,
		Full,
		Relaxed
	};

	namespace Mem
	{

		/// Uses acquire semantics (operation's effects are visible prior to any operation after)
		struct Acquire: public SMemorySemantic
		{
			static const EMemorySemantic value = EMemorySemantic::Acquire;
		};
		/// Uses release semantics (operation's effects are visible after any operation prior)
		struct Release: public SMemorySemantic
		{
			static const EMemorySemantic value = EMemorySemantic::Release;
		};
		/// Uses acquire and release (full) semantics (operation's effects are visible prior to any operation after and after any operation prior)
		struct Full: public SMemorySemantic
		{
			static const EMemorySemantic value = EMemorySemantic::Full;
		};
		/// No memory semantics are explicitly imposed (it doesn't matter when the operation's effects are visible as long as it happens eventually)
		struct Relaxed: public SMemorySemantic
		{
			static const EMemorySemantic value = EMemorySemantic::Relaxed;
		};

	}

}}
