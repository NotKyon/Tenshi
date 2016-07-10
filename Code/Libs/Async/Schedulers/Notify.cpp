#include "Notify.hpp"
#include "../Scheduler.hpp"
#include "../CPU.hpp"
#include "../../Core/Manager.hpp"

namespace Ax { namespace Async { namespace Detail_v1 {

#if AX_ASYNC_NOTIFY_ENABLED
	static const uint32				kMaxNotifies		= 65536;

	struct REvent
	{
		ENotify						Type;
		uint64						EnterCycle;
		uint64						LeaveCycle;

		inline uint64 ElapsedCycles() const
		{
			return LeaveCycle - EnterCycle;
		}
	};
	struct LThreadEvents
	{
		uint32						CurrentEvent;
		REvent						Events				[ kMaxNotifies ];
		volatile uint32				uFrameId;

		uint32						LargestEvent;
		uint64						LongestCycles;

		inline uint64 ElapsedCycles( uint32 EventId ) const
		{
			return Events[ EventId ].ElapsedCycles();
		}
	};
	struct MMediator
	{
		LThreadEvents				Locals				[ kMaxWorkers ];

		static MMediator &GetInstance();

	private:
		MMediator()
		{
			memset( &Locals, 0, sizeof( Locals ) );
		}
	};
	static TManager< MMediator >	MsgMgr;

	MMediator &MMediator::GetInstance()
	{
		static MMediator instance;
		return instance;
	}
	
	bool Notify( ENotify Type, ENotifyPhase Phase )
	{
		// CPU timestamp for when the event ended (or the notification was fired)
		const uint64 LeaveCycle = GetCPUCycles();

		// Current worker ID
		const uint32 uWorkerId = SchedulerCPU::GetCurrentWorkerId();
		if( uWorkerId >= kMaxWorkers ) {
			return false;
		}

		// Holder of the events for the current worker
		LThreadEvents &Events = MsgMgr->Locals[ uWorkerId ];

		// Special handling for frames
		if( Type == ENotify::Frame ) {
			if( Phase == ENotifyPhase::Enter ) {
				Events.LargestEvent = ~uint32( 0 );
				Events.CurrentEvent = 1;
				Events.Events[ 0 ].Type = ENotify::Frame;
				Events.Events[ 0 ].EnterCycle = LeaveCycle;
				Events.Events[ 0 ].LeaveCycle = 0;
			} else if( Phase == ENotifyPhase::Leave ) {
				Events.Events[ 0 ].LeaveCycle = LeaveCycle;
				//printf( "CPU%.2u Frame %.3u: LongestCycles=%I64u\n", uWorkerId, Events.uFrameId, Events.LongestCycles );
				AtomicInc( &Events.uFrameId, Mem::Release() );
			}

			AX_ASSERT_MSG( Phase != ENotifyPhase::Signal, "Invalid notify-phase for Frame" );
			return true;
		}

		// Pointer to the current event
		REvent *pEvent = NULL;

		// Look for a previous event if we're in a "leave" phase
		if( Phase == ENotifyPhase::Leave ) {
			for( uint32 uEvent = Events.CurrentEvent; uEvent > 0; --uEvent ) {
				const uint32 uIndex = uEvent - 1;

				if( Events.Events[ uIndex ].Type != Type ) {
					continue;
				}
				if( Events.Events[ uIndex ].LeaveCycle != 0 ) {
					continue;
				}

				pEvent = &Events.Events[ uIndex ];
				break;
			}

			// If the event wasn't found then give up
			if( !pEvent ) {
				AX_ASSERT_MSG( false, "Event not found for leave-phase notification" );
				return false;
			}
		} else {
			// If we can't allocate an event then give up
			if( Events.CurrentEvent == kMaxNotifies ) {
				return false;
			}

			// Allocate the event
			pEvent = &Events.Events[ Events.CurrentEvent++ ];
		}

		// Set the event's type
		pEvent->Type = Type;

		// Save timing information for the event
		if( Phase != ENotifyPhase::Enter ) {
			pEvent->LeaveCycle = LeaveCycle;
			if( Phase == ENotifyPhase::Leave ) {
				const uint64 Cycles = pEvent->LeaveCycle - pEvent->EnterCycle;
				if( Events.LargestEvent == ~uint32( 0 ) || Cycles > Events.ElapsedCycles( Events.LargestEvent ) ) {
					Events.LargestEvent = uint32( pEvent - &Events.Events[ 0 ] );
				}
				if( Events.LongestCycles < Cycles ) {
					Events.LongestCycles = Cycles;
				}
			}
		} else {
			pEvent->EnterCycle = GetCPUCycles();
			pEvent->LeaveCycle = 0;
		}

		// Done
		return true;
	}
#endif

}}}
