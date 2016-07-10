#pragma once

#include "../../Core/Types.hpp"

namespace Ax { namespace Async {

	/// Execution statistics
	struct SExecStats
	{
		/// Shortest amount of time any job took (microseconds)
		uint64						MinJobTime;
		/// Largest amount of time any job took (microseconds)
		uint64						MaxJobTime;
		/// Total amount of time across all threads that the jobs took to run (serial; microseconds)
		uint64						TotalJobTime;
		/// Total amount of time it took to run the jobs (parallel; microseconds)
		uint64						TotalRunTime;

		/// Constructor
		inline SExecStats()
		{
			Reset();
		}
	
		/// Reset to the default values
		inline void Reset()
		{
			MinJobTime = uint64( -1 );
			MaxJobTime = 0;
			TotalJobTime = 0;
			TotalRunTime = 0;
		}
		/// Update the stats with the settings of a single job
		inline void AddJobTime( uint64 microseconds )
		{
			if( !microseconds ) {
				microseconds = 1;
			}

			if( MinJobTime > microseconds ) {
				MinJobTime = microseconds;
			}
			if( MaxJobTime < microseconds ) {
				MaxJobTime = microseconds;
			}

			TotalJobTime += microseconds;
		}
		/// Merge another set of stats with this one
		inline SExecStats &Merge( const SExecStats &other )
		{
			if( MinJobTime > other.MinJobTime ) {
				MinJobTime = other.MinJobTime;
			}
			if( MaxJobTime < other.MaxJobTime ) {
				MaxJobTime = other.MaxJobTime;
			}

			TotalJobTime += other.TotalJobTime;
			if( TotalRunTime < other.TotalRunTime ) {
				TotalRunTime = other.TotalRunTime;
			}

			return *this;
		}
		/// Output the stats to the logger
		void OutputDebugInfo( const char *pszName = nullptr ) const;
	};

	template< uint32 tSize >
	inline void ResetExecStats( SExecStats( &stats )[ tSize ] )
	{
		for( uint32 i = 0; i < tSize; ++i ) {
			stats[ i ].Reset();
		}
	}
	template< uint32 tSize >
	inline void MergeExecStats( SExecStats &main, SExecStats( &stats )[ tSize ] )
	{
		for( uint32 i = 0; i < tSize; ++i ) {
			main.Merge( stats[ i ] );
		}
	}
	template< uint32 tSize >
	inline void MergeAndResetExecStats( SExecStats &main, SExecStats( &stats )[ tSize ] )
	{
		for( uint32 i = 0; i < tSize; ++i ) {
			main.Merge( stats[ i ] );
			stats[ i ].Reset();
		}
	}

}}
