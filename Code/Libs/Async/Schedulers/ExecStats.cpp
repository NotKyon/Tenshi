#include "ExecStats.hpp"

#include "../../Core/String.hpp"
#include "../../Core/Logger.hpp"

#include "../../System/TimeConversion.hpp"

namespace Ax { namespace Async {

	void SExecStats::OutputDebugInfo( const char *pszName ) const
	{
		String s;
		
		s.Reserve( 512 );
		s = "ExecStats";

		if( pszName != nullptr && *pszName != '\0' ) {
			s += " <";
			s += pszName;
			s += ">\n";
		} else {
			s += ":\n";
		}

		const double minJobTimeSec = System::MicrosecondsToSeconds( MinJobTime );
		const double minJobTimeFrm = System::MicrosecondsToFrames( MinJobTime );

		const double maxJobTimeSec = System::MicrosecondsToSeconds( MaxJobTime );
		const double maxJobTimeFrm = System::MicrosecondsToFrames( MaxJobTime );

		const double totalJobTimeSec = System::MicrosecondsToSeconds( TotalJobTime );
		const double totalJobTimeFrm = System::MicrosecondsToFrames( TotalJobTime );

		const double totalRunTimeSec = System::MicrosecondsToSeconds( TotalRunTime );
		const double totalRunTimeFrm = System::MicrosecondsToFrames( TotalRunTime );

		s.AppendFormat( "  minJobTime   = %.6f sec [%.3f%%]\n", minJobTimeSec, minJobTimeFrm*100.0 );
		s.AppendFormat( "  maxJobTime   = %.6f sec [%.3f%%]\n", maxJobTimeSec, maxJobTimeFrm*100.0 );
		s.AppendFormat( "  totalJobTime = %.6f sec [%.3f%%]\n", totalJobTimeSec, totalJobTimeFrm*100.0 );
		s.AppendFormat( "  totalRunTime = %.6f sec [%.3f%%]\n", totalRunTimeSec, totalRunTimeFrm*100.0 );

		if( s.EndsWith( "\n" ) ) {
			s.Remove( -1, 1 );
		}
		BasicStatusf( "%s", s.CString() );
	}

}}
