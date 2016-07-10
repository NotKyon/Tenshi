#pragma once

#include "ExecStats.hpp"
#include "../CPU.hpp"

#include "../../Core/Assert.hpp"
#include "../../Core/String.hpp"

#ifndef AX_PROFILING_ENABLED
# if AX_DEVELOPER_BUILD
#  define AX_PROFILING_ENABLED 1
# else
#  define AX_PROFILING_ENABLED 0
# endif
#endif

namespace Ax { namespace Async {

	static const uint32				kMaxProfileEventsPerPool = 16384;

	struct RProfileEvent
	{
		const char *				pszName; //e.g., __func__ or "Entity Update - %s"
		const char *				pszParm; //used to replace "%s" in display
		uint64						HeadCPUTimestamp;
		uint64						TailCPUTimestamp;
		RProfileEvent *				pSubeventHead;
		RProfileEvent *				pSubeventTail;
		RProfileEvent *				pNextEvent;
		RProfileEvent *				pParentEvent;

		inline String GetName() const
		{
			return String( pszName ).Replaced( "%s", pszParm != nullptr ? pszParm : "" );
		}
		inline uint64 GetElapsedTimestamps() const
		{
			return TailCPUTimestamp - HeadCPUTimestamp;
		}
		inline double GetElapsedFrameTime( uint64 timestampsPerFrame ) const
		{
			return double( GetElapsedTimestamps() )/double( timestampsPerFrame );
		}
	};
	struct LProfileEventPool
	{
		RProfileEvent				Events[ kMaxProfileEventsPerPool ];
		RProfileEvent *				pCurrentEvent;
		uint32						cEvents;
#if AX_DEBUG_ENABLED
# if defined( _WIN32 )
		DWORD						dwThreadId;
# endif
#endif

		LProfileEventPool();
		~LProfileEventPool();

		RProfileEvent *AllocEvent();

		inline void PushEvent( RProfileEvent *pScope )
		{
			pCurrentEvent = pScope;
		}
		inline void PopEvent()
		{
			pCurrentEvent = ( pCurrentEvent != nullptr ) ? pCurrentEvent->pParentEvent : nullptr;
		}

		inline void Reset()
		{
			cEvents = 0;
			pCurrentEvent = nullptr;
		}

		static void Enter( RProfileEvent *pEvent, const char *pszName = nullptr, const char *pszParm = nullptr );
		static void Leave( RProfileEvent *pEvent );
	};

	LProfileEventPool &GetProfilePool();
	void ResetAllProfilePools();

	inline void LProfileEventPool::Enter( RProfileEvent *pEvent, const char *pszName, const char *pszParm )
	{
		AX_ASSERT_NOT_NULL( pEvent );

		pEvent->pszName = pszName;
		pEvent->pszParm = pszParm;
		GetProfilePool().PushEvent( pEvent );
		pEvent->HeadCPUTimestamp = GetCPUCycles();
	}
	inline void LProfileEventPool::Leave( RProfileEvent *pEvent )
	{
		AX_ASSERT_NOT_NULL( pEvent );

		pEvent->TailCPUTimestamp = GetCPUCycles();
		GetProfilePool().PopEvent();
	}

	class CProfileSampler
	{
	public:
		inline CProfileSampler()
		: m_pEvent( nullptr )
		{
		}
		inline ~CProfileSampler()
		{
			Leave();
		}

		inline void Enter( const char *pszName = nullptr, const char *pszParm = nullptr )
		{
			m_pEvent = GetProfilePool().AllocEvent();
			if( !m_pEvent ) {
				return;
			}

			LProfileEventPool::Enter( m_pEvent, pszName, pszParm );
		}
		inline void Leave()
		{
			if( !m_pEvent ) {
				return;
			}

			LProfileEventPool::Leave( m_pEvent );
			m_pEvent = nullptr;
		}

	private:
		RProfileEvent *				m_pEvent;
	};

	class CScopedProfileSampler
	{
	public:
		inline CScopedProfileSampler( const char *pszName = nullptr, const char *pszParm = nullptr )
		: m_pEvent( GetProfilePool().AllocEvent() )
		{
			if( !m_pEvent ) {
				return;
			}

			LProfileEventPool::Enter( m_pEvent );
		}
		inline ~CScopedProfileSampler()
		{
			if( !m_pEvent ) {
				return;
			}

			LProfileEventPool::Leave( m_pEvent );
		}

	private:
		RProfileEvent *const			m_pEvent;
	};

#if AX_PROFILING_ENABLED
# define AX_SCOPE_PROFILE()								Ax::Async::CScopedProfileSampler AX_ANONVAR(ScopeProfiler)(AX_PRETTY_FUNCTION)
# define AX_NAMED_SCOPE_PROFILE(Name)					Ax::Async::CScopedProfileSampler AX_ANONVAR(ScopeProfiler)(Name)
# define AX_NAMED_SCOPE_PROFILE_EX(Name,Parm)			Ax::Async::CScopedProfileSampler AX_ANONVAR(ScopeProfiler)(Name,Parm)
#else
# define AX_SCOPE_PROFILE()								((void)0)
# define AX_NAMED_SCOPE_PROFILE(Name)					((void)0)
# define AX_NAMED_SCOPE_PROFILE_EX(Name,Parm)			((void)0)
#endif

}}
