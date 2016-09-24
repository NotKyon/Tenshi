#pragma once

#include "TypeTraits.hpp"

#if !AX_CXX_N2118
# error [Core::ScopeGuard] Requires a C++11-compliant compiler (r-value references)
#endif

namespace Ax
{

	/*!
	 *	ScopeGuard
	 *
	 *	Executes a given function when it exits scope unless it's told not to.
	 */
	template< class Functor >
	class ScopeGuard
	{
	public:
		/*! Primary constructor */
		inline ScopeGuard( const Functor &func )
		: m_pfnCallback( func )
		, m_State( State::Valid )
		{
		}
		/*! Move constructor */
		inline ScopeGuard( ScopeGuard< Functor > &&r )
		: m_pfnCallback( r.m_pfnCallback )
		, m_State( r.m_State )
		{
			r.Commit();
		}
		/*! Destructor */
		inline ~ScopeGuard()
		{
			Call();
		}

		/*! Set the state to "committed" to prevent function from executing */
		inline void Commit()
		{
			m_State = State::Committed;
		}
		/*! Invoke the function immediately */
		inline void Call()
		{
			if( m_State != State::Valid ) {
				return;
			}

			m_pfnCallback();
			Commit();
		}

		/*! Invoke the function immediately */
		inline void operator()()
		{
			Call();
		}

	private:
		enum class State
		{
			Valid,
			Committed
		};

		const Functor				m_pfnCallback;
		State						m_State;
	};

	template< class Functor >
	inline ScopeGuard< Functor > MakeScopeGuard( const Functor &func )
	{
		//return Forward< ScopeGuard< Functor > >( func );
		return ScopeGuard< Functor >( func );
	}

}

#define AX_SCOPE_GUARD( lambda )\
	auto AX_ANONVAR( ScopeGuard__ ) = Ax::MakeScopeGuard( [ & ]()lambda )
