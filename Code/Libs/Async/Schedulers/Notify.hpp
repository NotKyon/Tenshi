#pragma once

#include "../../Core/Types.hpp"
#include "../../Platform/Platform.hpp"

#ifndef AX_ASYNC_NOTIFY_ENABLED
# if AX_DEBUG_ENABLED
#  define AX_ASYNC_NOTIFY_ENABLED	1
# else
#  define AX_ASYNC_NOTIFY_ENABLED	0
# endif
#endif

namespace Ax { namespace Async {

	namespace Detail_v1 {

		enum class ENotify : uint32
		{
			/// Per-frame ("per-tick") update [enter/leave]
			Frame,
			/// Retrieve a job from the queue [enter/leave]
			FetchJob,
			/// Execute the retrieved job [enter/leave]
			ExecuteJob,
			/// Update execution statistics [enter/leave]
			UpdateExecStats,
			/// Submit work that has been waiting to be submitted [enter/leave]
			DispatchDeferredWork,
			/// Update settings for the last job chain [enter/leave]
			UpdateJobChain,
			/// Submit work to the scheduler [enter/leave]
			SubmitWork,
			/// Worker-thread execution until no work is available [enter/leave]
			ExecuteUntilNoWork,
			/// Time to acquire a resource [enter/leave]
			AcquireResource,
			/// Time to release a resource [enter/leave]
			ReleaseResource,
			/// Time spent waiting for a resource [enter/leave]
			WaitResource,
			/// Time spent waiting for work to become available [enter/leave]
			WaitWork,
			/// A resource was successfully acquired [signal]
			ResourceAcquired,
			/// A resource was not acquired [signal]
			ResourceNotAcquired,
			/// A resource was released [signal]
			ResourceReleased,
			/// A job was successfully retrieved [signal]
			JobFetched,
			/// A job could not be retrieved [signal]
			JobNotFetched,
			/// Spent time doing nothing (idling) [signal]
			IdleSpin,
		};
		enum class ENotifyPhase : uint32
		{
			Signal,
			Enter,
			Leave
		};

#if AX_ASYNC_NOTIFY_ENABLED
		bool Notify( ENotify Type, ENotifyPhase Phase = ENotifyPhase::Signal );
#else
		AX_FORCEINLINE bool Notify( ENotify, ENotifyPhase = ENotifyPhase::Signal )
		{
			return false;
		}
#endif

		template< ENotify tType >
		class TScopeNotify
		{
		public:
			AX_FORCEINLINE TScopeNotify()
			: m_bDidEnter( false )
			{
				m_bDidEnter = Notify( tType, ENotifyPhase::Enter );
			}
			AX_FORCEINLINE ~TScopeNotify()
			{
				if( !m_bDidEnter ) {
					return;
				}

				Notify( tType, ENotifyPhase::Leave );
			}

		private:
			bool					m_bDidEnter;
		};

		template< ENotify tType >
		class TNotify
		{
		public:
			AX_FORCEINLINE TNotify()
			: m_cEnters( 0 )
			, m_cFailed( 0 )
			{
			}

			AX_FORCEINLINE void Enter()
			{
				if( m_cFailed > 0 ) {
					++m_cFailed;
					return;
				}

				const bool bDidEnter = Notify( tType, ENotifyPhase::Enter );
				if( bDidEnter ) {
					++m_cEnters;
				} else {
					++m_cFailed;
				}
			}
			AX_FORCEINLINE void Leave()
			{
				if( m_cFailed > 0 ) {
					--m_cFailed;
					return;
				}

				Notify( tType, ENotifyPhase::Leave );
				--m_cEnters;
			}

		private:
			uint32				m_cEnters;
			uint32				m_cFailed;
		};

	}

	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::Frame >				CAutoNotifyFrame;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::FetchJob >				CAutoNotifyFetchJob;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::ExecuteJob >			CAutoNotifyExecuteJob;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::UpdateExecStats >		CAutoNotifyUpdateExecStats;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::DispatchDeferredWork >	CAutoNotifyDispatchDeferredWork;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::UpdateJobChain >		CAutoNotifyUpdateJobChain;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::SubmitWork >			CAutoNotifySubmitWork;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::ExecuteUntilNoWork >	CAutoNotifyExecuteUntilNoWork;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::AcquireResource >		CAutoNotifyAcquireResource;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::ReleaseResource >		CAutoNotifyReleaseResource;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::WaitResource >			CAutoNotifyWaitResource;
	typedef Detail_v1::TScopeNotify< Detail_v1::ENotify::WaitWork >				CAutoNotifyWaitWork;

	typedef Detail_v1::TNotify< Detail_v1::ENotify::Frame >						CNotifyFrame;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::FetchJob >					CNotifyFetchJob;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::ExecuteJob >				CNotifyExecuteJob;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::UpdateExecStats >			CNotifyUpdateExecStats;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::DispatchDeferredWork >		CNotifyDispatchDeferredWork;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::UpdateJobChain >			CNotifyUpdateJobChain;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::SubmitWork >				CNotifySubmitWork;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::ExecuteUntilNoWork >		CNotifyExecuteUntilNoWork;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::AcquireResource >			CNotifyAcquireResource;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::ReleaseResource >			CNotifyReleaseResource;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::WaitResource >				CNotifyWaitResource;
	typedef Detail_v1::TNotify< Detail_v1::ENotify::WaitWork >					CNotifyWaitWork;

	AX_FORCEINLINE void NotifyResourceAcquired()
	{
		Detail_v1::Notify( Detail_v1::ENotify::ResourceAcquired );
	}
	AX_FORCEINLINE void NotifyResourceNotAcquired()
	{
		Detail_v1::Notify( Detail_v1::ENotify::ResourceNotAcquired );
	}
	AX_FORCEINLINE void NotifyResourceReleased()
	{
		Detail_v1::Notify( Detail_v1::ENotify::ResourceReleased );
	}
	AX_FORCEINLINE void NotifyJobFetched()
	{
		Detail_v1::Notify( Detail_v1::ENotify::JobFetched );
	}
	AX_FORCEINLINE void NotifyJobNotFetched()
	{
		Detail_v1::Notify( Detail_v1::ENotify::JobNotFetched );
	}
	AX_FORCEINLINE void NotifyIdleSpin()
	{
		Detail_v1::Notify( Detail_v1::ENotify::IdleSpin );
	}

}}
