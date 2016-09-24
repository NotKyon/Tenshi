#include "CPU.hpp"
#include "Detail/Threading.hpp"
#include "Thread.hpp"
#include "../Platform/Platform.hpp"
#include "../Core/Manager.hpp"
#if defined( _MSC_VER )
# include <intrin.h>
#endif

#ifndef AX_CPU_PAUSE
# if ( defined( _MSC_VER ) && _MSC_VER >= 1300 ) || defined( __INTEL_COMPILER )
#  pragma intrinsic( _mm_pause )
#  define AX_CPU_PAUSE() _mm_pause()
# elif ( defined( __GNUC__ ) || defined( __clang__ ) ) && ( AX_ARCH & AX_ARCH_INTEL )
#  define AX_CPU_PAUSE() __asm__ __volatile__( "pause;" )
# else
#  error AX_CPU_PAUSE: Unhandled compiler or architecture
# endif
#endif

namespace Ax { namespace Async {

#if ( AX_ARCH & AX_ARCH_INTEL )
	namespace X86
	{

		void CPUID( uint32( &CPUInfo )[ 4 ], uint32 FunctionInfo, uint32 SubfunctionInfo = 0 )
		{
# if defined( _MSC_VER ) || defined( __INTEL_COMPILER )
			__cpuidex( ( int * )CPUInfo, FunctionInfo, SubfunctionInfo );
# elif defined( __GNUC__ ) || defined( __clang__ )
			__asm__ __volatile__
			(
				"cpuid"
				:
					"=a" (CPUInfo[0]),
					"=b" (CPUInfo[1]),
					"=c" (CPUInfo[2]),
					"=d" (CPUInfo[3])
				:
					"a" (FunctionInfo),
					"c" (SubfunctionInfo),
					"d" (0)
			);
# else
#  error CPUID not supported on this compiler
# endif
		}

		enum class ECPUVendor : uint32
		{
			Unknown,
			Intel,
			AMD
		};

		struct SCPUInfo
		{
			char					szBrand				[ 48 ];
			char					szVendor			[ 16 ];
			ECPUVendor				Vendor;
			uint32					uLastNormalId;
			uint32					uLastExtendedId;

			uint32					uF01ECX;
			uint32					uF01EDX;
			uint32					uF07EBX;
			uint32					uF07ECX;
			uint32					uF81ECX;
			uint32					uF81EDX;

			static SCPUInfo &GetInstance();

			inline bool F01ECX( uint32 Index ) const
			{
				return ( ( uF01ECX>>Index ) & 1 ) == 1;
			}
			inline bool F01EDX( uint32 Index ) const
			{
				return ( ( uF01EDX>>Index ) & 1 ) == 1;
			}
			inline bool F07EBX( uint32 Index ) const
			{
				return ( ( uF07EBX>>Index ) & 1 ) == 1;
			}
			inline bool F07ECX( uint32 Index ) const
			{
				return ( ( uF07ECX>>Index ) & 1 ) == 1;
			}
			inline bool F81ECX( uint32 Index ) const
			{
				return ( ( uF81ECX>>Index ) & 1 ) == 1;
			}
			inline bool F81EDX( uint32 Index ) const
			{
				return ( ( uF81EDX>>Index ) & 1 ) == 1;
			}

			inline bool IsIntel() const					{ return Vendor == ECPUVendor::Intel; }
			inline bool IsAMD() const					{ return Vendor == ECPUVendor::AMD; }

			inline bool SSE3() const					{ return F01ECX( 0 ); }
			inline bool PCLMULQDQ() const				{ return F01ECX( 1 ); }
			inline bool MONITOR() const					{ return F01ECX( 3 ); }
			inline bool SSSE3() const					{ return F01ECX( 9 ); }
			inline bool FMA() const						{ return F01ECX( 12 ); }
			inline bool CMPXCHG16B() const				{ return F01ECX( 13 ); }
			inline bool SSE41() const					{ return F01ECX( 19 ); }
			inline bool SSE42() const					{ return F01ECX( 20 ); }
			inline bool MOVBE() const					{ return F01ECX( 22 ); }
			inline bool POPCNT() const					{ return F01ECX( 23 ); }
			inline bool AES() const						{ return F01ECX( 25 ); }
			inline bool XSAVE() const					{ return F01ECX( 26 ); }
			inline bool OSXSAVE() const					{ return F01ECX( 27 ); }
			inline bool AVX() const						{ return F01ECX( 28 ); }
			inline bool F16C() const					{ return F01ECX( 29 ); }
			inline bool RDRAND() const					{ return F01ECX( 30 ); }

			inline bool MSR() const						{ return F01EDX( 5 ); }
			inline bool CX8() const						{ return F01EDX( 8 ); }
			inline bool SEP() const						{ return F01EDX( 11 ); }
			inline bool CMOV() const					{ return F01EDX( 15 ); }
			inline bool CLFSH() const					{ return F01EDX( 19 ); }
			inline bool MMX() const						{ return F01EDX( 23 ); }
			inline bool FXSR() const					{ return F01EDX( 24 ); }
			inline bool SSE() const						{ return F01EDX( 25 ); }
			inline bool SSE2() const					{ return F01EDX( 26 ); }
			inline bool HTT() const						{ return IsIntel() && F01EDX( 28 ); }

			inline bool FSGSBASE() const				{ return F07EBX( 0 ); }
			inline bool BMI1() const					{ return F07EBX( 3 ); }
			inline bool HLE() const						{ return IsIntel() && F07EBX( 4 ); }
			inline bool AVX2() const					{ return F07EBX( 5 ); }
			inline bool BMI2() const					{ return F07EBX( 8 ); }
			inline bool ERMS() const					{ return F07EBX( 9 ); }
			inline bool INVPCID() const					{ return F07EBX( 10 ); }
			inline bool RTM() const						{ return IsIntel() && F07EBX( 11 ); }
			inline bool AVX512F() const					{ return F07EBX( 16 ); }
			inline bool RDSEED() const					{ return F07EBX( 18 ); }
			inline bool ADX() const						{ return F07EBX( 19 ); }
			inline bool AVX512PF() const				{ return F07EBX( 26 ); }
			inline bool AVX512ER() const				{ return F07EBX( 27 ); }
			inline bool AVX512CD() const				{ return F07EBX( 28 ); }
			inline bool SHA() const						{ return F07EBX( 29 ); }

			inline bool PREFETCHWT1() const				{ return F07ECX( 0 ); }

			inline bool LAHF() const					{ return F81ECX( 0 ); }
			inline bool LZCNT() const					{ return IsIntel() && F81ECX( 5 ); }
			inline bool ABM() const						{ return IsAMD() && F81ECX( 5 ); }
			inline bool SSE4a() const					{ return IsAMD() && F81ECX( 6 ); }
			inline bool XOP() const						{ return IsAMD() && F81ECX( 11 ); }
			inline bool TBM() const						{ return IsAMD() && F81ECX( 21 ); }

			inline bool SYSCALL() const					{ return IsIntel() && F81EDX( 11 ); }
			inline bool MMXEXT() const					{ return IsAMD() && F81EDX( 22 ); }
			inline bool RDTSCP() const					{ return IsIntel() && F81EDX( 27 ); }
			inline bool AMD3DNOWEXT() const				{ return IsAMD() && F81EDX( 30 ); }
			inline bool AMD3DNOW() const				{ return IsAMD() && F81EDX( 31 ); }

		private:
			inline SCPUInfo()
			: Vendor( ECPUVendor::Unknown )
			, uLastNormalId( 0 )
			, uLastExtendedId( 0 )
			, uF01ECX( 0 )
			, uF01EDX( 0 )
			, uF07EBX( 0 )
			, uF07ECX( 0 )
			, uF81ECX( 0 )
			, uF81EDX( 0 )
			{
				szVendor[ 0 ] = '\0';
				szBrand[ 0 ] = '\0';

				enum { EAX=0, EBX=1, ECX=2, EDX=3 };
				uint32 Regs[ 4 ];

				// CPUID( EAX=00h ) �� EAX: highest function number; EBX,ECX,EDX: vendor string
				CPUID( Regs, 0 );
				uLastNormalId = Regs[ EAX ];
				*( uint32 * )&szVendor[ 0x00 ] = Regs[ EBX ];
				*( uint32 * )&szVendor[ 0x04 ] = Regs[ ECX ];
				*( uint32 * )&szVendor[ 0x08 ] = Regs[ EDX ];
				*( uint32 * )&szVendor[ 0x0C ] = 0;

				if( strncmp( szVendor, "GenuineIntel", 12 ) == 0 ) {
					Vendor = ECPUVendor::Intel;
				} else if( strncmp( szVendor, "AuthenticAMD", 12 ) == 0 ) {
					Vendor = ECPUVendor::AMD;
				}

				// Get function 01h
				if( uLastNormalId >= 0x01 ) {
					CPUID( Regs, 0x01 );

					uF01ECX = Regs[ ECX ];
					uF01EDX = Regs[ EDX ];
				}

				// Get function 07h
				if( uLastNormalId >= 0x07 ) {
					CPUID( Regs, 0x07 );

					uF07EBX = Regs[ EBX ];
				}

				// CPUID( EAX=80000000h ) �� EAX: highest extended function number
				CPUID( Regs, 0x80000000 );
				uLastExtendedId = Regs[ EAX ];

				// Get extended function 01h
				if( uLastExtendedId >= 0x80000001 ) {
					CPUID( Regs, 0x80000001 );
					uF81ECX = Regs[ ECX ];
					uF81EDX = Regs[ EDX ];
				}

				// Get brand
				if( uLastExtendedId >= 0x80000004 ) {
					CPUID( Regs, 0x80000002 );
					*( uint32 * )&szBrand[ 0x00 ] = Regs[ EAX ];
					*( uint32 * )&szBrand[ 0x04 ] = Regs[ EBX ];
					*( uint32 * )&szBrand[ 0x08 ] = Regs[ ECX ];
					*( uint32 * )&szBrand[ 0x0C ] = Regs[ EDX ];

					CPUID( Regs, 0x80000003 );
					*( uint32 * )&szBrand[ 0x10 ] = Regs[ EAX ];
					*( uint32 * )&szBrand[ 0x14 ] = Regs[ EBX ];
					*( uint32 * )&szBrand[ 0x18 ] = Regs[ ECX ];
					*( uint32 * )&szBrand[ 0x1C ] = Regs[ EDX ];

					CPUID( Regs, 0x80000004 );
					*( uint32 * )&szBrand[ 0x20 ] = Regs[ EAX ];
					*( uint32 * )&szBrand[ 0x24 ] = Regs[ EBX ];
					*( uint32 * )&szBrand[ 0x28 ] = Regs[ ECX ];
					*( uint32 * )&szBrand[ 0x2C ] = Regs[ EDX ];

					*( uint32 * )&szBrand[ 0x30 ] = 0;
				}
			}
			
			SCPUInfo( const SCPUInfo & ) AX_DELETE_FUNC;
			SCPUInfo &operator=( const SCPUInfo & ) AX_DELETE_FUNC;
		};
		SCPUInfo &SCPUInfo::GetInstance()
		{
			static SCPUInfo instance;
			return instance;
		}
		static TManager< SCPUInfo >	CPUInfo;
	}
#endif

	const char *GetCPUVendor()
	{
#if AX_ARCH & AX_ARCH_INTEL
		return X86::CPUInfo->szVendor;
#else
		AX_ASSERT_MSG( false, "Not implemented" );
		return "";
#endif
	}
	const char *GetCPUBrand()
	{
#if AX_ARCH & AX_ARCH_INTEL
		return X86::CPUInfo->szBrand;
#else
		AX_ASSERT_MSG( false, "Not implemented" );
		return "";
#endif
	}

	bool IsHyperthreadingAvailable()
	{
#if AX_ARCH & AX_ARCH_INTEL
		static bool didInit = false;
		static bool htAvail;

		if( !didInit ) {
# if 0
			uint32 d = 0;

			// Get the feature set
#  if defined( __GNUC__ ) || defined( __clang__ )
			__asm__ __volatile__("cpuid" : "=d" (d) : "a" (1), "d" (0));
#  elif defined( _MSC_VER )
			int regs[ 4 ] = { 0, 0, 0, 0 };
			__cpuid( regs, 1 );
			d = regs[ 3 ];
#  endif

			htAvail = !!( ( d>>28 ) & 1 );
			didInit = true;
# else
			htAvail = X86::CPUInfo->HTT();
			didInit = true;
# endif
		}

		return htAvail;
#else
		return false;
#endif
	}
	uint32 GetCPUCoreCount()
	{
#if defined( _WIN32 )
		SYSTEM_INFO si;

		memset(&si, 0, sizeof(si));
		GetSystemInfo(&si);

		return (uint32)si.dwNumberOfProcessors;
#elif AX_PLATFORM & AX_PLATFORM_LINUX
		int count;

		if( ( count = sysconf( _SC_NPROCESSORS_ONLN ) ) < 1 ) {
			AX_WARNING_LOG += "sysconf(_SC_NPROCESSORS_ONLN) failed! Defaulting to 1";

			count = 1;
		}

		return ( uint32 )count;
#elif AX_PLATFORM & AX_PLATFORM_MAC
		//
		//	TODO!
		//

		AX_DEBUG_LOG += "Not implemented";
		return 1;
#else
		return 1;
#endif
	}
	uint32 GetCPUThreadCount()
	{
		return GetCPUCoreCount()*( 1 + ( +IsHyperthreadingAvailable() ) );
	}

	uint64 GetCPUCycles()
	{
#if defined( _MSC_VER )
# if defined( _M_IX86 ) || defined( _M_X64 )
		return __rdtsc();
# else
#  error Unhandled architecture for GetCPUCycles()
# endif
#elif defined( __GNUC__ ) || defined( __clang__ )
# if defined( __x64__ ) || defined( __x86_64__ ) || defined( __amd64__ )
		uint64 cycles = 0;
		__asm__ __volatile__( "rdtsc;" : "=a" (cycles) );
		return cycles;
# elif defined( __x86__ ) || defined( __i386__ )
		uint32 a, b;
		__asm__ __volatile__( "rdtsc;" : "=a" (a), "=d" (b) );
		const uint64 cycles = ( uint64(a)<<32 ) | uint64(b);
		return cycles; 
# else
#  error Unhandled architecture for GetCPUCycles()
# endif
#else
# error Unhandled compiler for GetCPUCycles()
#endif
	}

	void CPUPause( uint32 uDelay )
	{
		for( uint32 i = 0; i < uDelay; ++i ) {
			AX_CPU_PAUSE();
		}
	}

	void Backoff( uint32 &cSpins, uint32 cMaxSpins )
	{
		if( cSpins > cMaxSpins ) {
			// Provide work to something else because we're taking too long
			CThread::Yield();
			return;
		}

		// Wait for a little while
		LocalSpin( cSpins );
		cSpins = cSpins*2 - cSpins/2; //Safe integer equivalent of multiplication by 1.5
	}

}}
