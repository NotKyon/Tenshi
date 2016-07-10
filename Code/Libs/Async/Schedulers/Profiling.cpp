#include "Profiling.hpp"
#include "../Scheduler.hpp"

namespace Ax { namespace Async {

	struct MProfiler
	{
		LProfileEventPool Pools[ kMaxWorkers ];

		static MProfiler &GetInstance()
		{
			static MProfiler instance;
			return instance;
		}

	private:
		MProfiler()
		{
			memset( &Pools[ 0 ], 0, sizeof( Pools ) );
		}
	};

	LProfileEventPool &GetProfilePool()
	{
		const uint32 workerId = SchedulerCPU::GetCurrentWorkerId();
		AX_ASSERT( workerId < kMaxWorkers );

		auto &profiles = MProfiler::GetInstance();

#if AX_DEBUG_ENABLED
# if defined( _WIN32 )
		if( profiles.Pools[ workerId ].dwThreadId == 0 ) {
			profiles.Pools[ workerId ].dwThreadId = GetCurrentThreadId();
		}
# endif
#endif

		return profiles.Pools[ workerId ];
	}
	void ResetAllProfilePools()
	{
		auto &profiles = MProfiler::GetInstance();

		const uint32 cWorkers = SchedulerCPU::NumWorkers();
		for( uint32 i = 0; i < cWorkers; ++i ) {
			profiles.Pools[ i ].Reset();
		}
	}

	LProfileEventPool::LProfileEventPool()
	: pCurrentEvent( nullptr )
	, cEvents( 0 )
#if AX_DEBUG_ENABLED
# if defined( _WIN32 )
	, dwThreadId( 0 )
# endif
#endif
	{
	}
	LProfileEventPool::~LProfileEventPool()
	{
	}
	RProfileEvent *LProfileEventPool::AllocEvent()
	{
		// Only allowed to access this pool from the given thread!
#if AX_DEBUG_ENABLED
# if defined( _WIN32 )
		AX_ASSERT( GetCurrentThreadId() == dwThreadId );
# endif
#endif

		if( cEvents >= kMaxProfileEventsPerPool ) {
			return nullptr;
		}

		RProfileEvent *const pEvent = &Events[ cEvents++ ];
			
		pEvent->pszName = nullptr;
		pEvent->pszParm = nullptr;
		pEvent->HeadCPUTimestamp = 0;
		pEvent->TailCPUTimestamp = 0;
		pEvent->pSubeventHead = nullptr;
		pEvent->pSubeventTail = nullptr;
		pEvent->pNextEvent = nullptr;

		pEvent->pParentEvent = pCurrentEvent;
		if( pCurrentEvent != nullptr ) {
			if( pCurrentEvent->pSubeventTail != nullptr ) {
				pCurrentEvent->pSubeventTail->pNextEvent = pEvent;
			} else {
				pCurrentEvent->pSubeventHead = pEvent;
			}
			pCurrentEvent->pSubeventTail = pEvent;
		}

		return pEvent;
	}

}}
