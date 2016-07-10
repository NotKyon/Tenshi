#pragma once

namespace Ax
{

	template< typename TFirst, typename TSecond >
	struct TPair
	{
		typedef TFirst				FirstType;
		typedef TSecond				SecondType;
		typedef TPair<TFirst,TSecond>	ThisType;

		TFirst						First;
		TSecond						Second;

		inline TPair()
		: First()
		, Second()
		{
		}
		inline TPair( const TPair &X )
		: First( X.First )
		, Second( X.Second )
		{
		}
		template< typename TOtherFirst, typename TOtherSecond >
		inline TPair( const TPair< TOtherFirst, TOtherSecond > &X )
		: First( X.First )
		, Second( X.Second )
		{
		}
		inline TPair( const TFirst &A, const TSecond &B )
		: First( A )
		, Second( B )
		{
		}
	};

	template< typename TFirst, typename TSecond >
	inline TPair< TFirst, TSecond > MakePair( const TFirst &A, const TSecond &B )
	{
		return TPair< TFirst, TSecond >( A, B );
	}

}
