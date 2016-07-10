#pragma once

/*
===============================================================================

	COMPILER DATE/TIME (INTEGERS)

===============================================================================
*/

/// \def AX_DateCharLit__(c)
/// \brief Converts a single character in the __DATE__ string to an integer
#define AX_DateCharLit__( c )\
	( ( ( c ) == ' ' ) ? ( 0 ) : ( ( c ) - '0' ) )
/// \def AX_StrLitToInt__(strLit,index0,index1)
/// \brief Converts a set of two characters in the __DATE__ string to a
///        (potentially) two-digit integer
#define AX_StrLitToInt__( strLit, index0, index1 )\
	( AX_DateCharLit__( +strLit[ index0 ] )*10 + ( +strLit[ index1 ] - '0' ) )
/// \def AX_DoDateMonth__(a,b,c,code)
/// \brief If the first three letters of __DATE__ match the passed a, b, and c,
///        then the integer specified in code is returned, or 0 if not.
#define AX_DoDateMonth__( a,b,c, code )\
	( ( __DATE__[ 0 ] == a && __DATE__[ 1 ] == b && __DATE__[ 2 ] == c ) ?\
		code : 0 )

/// \def AX_INT_YEAR
/// \brief Retrieves the current year as an integer (y2k compatible!)
#define AX_INT_YEAR\
	( AX_StrLitToInt__( __DATE__, 9, 10 ) )
/// \def AX_INT_MONTH
/// \brief Retrieves the current month as an integer
#define AX_INT_MONTH (\
	AX_DoDateMonth__( 'J','a','n', 1 ) +\
	AX_DoDateMonth__( 'F','e','b', 2 ) +\
	AX_DoDateMonth__( 'M','a','r', 3 ) +\
	AX_DoDateMonth__( 'A','p','r', 4 ) +\
	AX_DoDateMonth__( 'M','a','y', 5 ) +\
	AX_DoDateMonth__( 'J','u','n', 6 ) +\
	AX_DoDateMonth__( 'J','u','l', 7 ) +\
	AX_DoDateMonth__( 'A','u','g', 8 ) +\
	AX_DoDateMonth__( 'S','e','p', 9 ) +\
	AX_DoDateMonth__( 'O','c','t', 10 ) +\
	AX_DoDateMonth__( 'N','o','v', 11 ) +\
	AX_DoDateMonth__( 'D','e','c', 12 ) )
/// \def AX_INT_DAY
/// \brief Retrieves the current day of the month as an integer
#define AX_INT_DAY\
	( AX_StrLitToInt__( __DATE__, 4, 5 ) )

/// \def AX_INT_HOUR
/// \brief The current hour (24 hour format) as an integer
#define AX_INT_HOUR\
	( AX_StrLitToInt__( __TIME__, 0, 1 ) )
/// \def AX_INT_MINUTE
/// \brief The current minute of the hour as an integer
#define AX_INT_MINUTE\
	( AX_StrLitToInt__( __TIME__, 3, 4 ) )
/// \def AX_INT_SECOND
/// \brief The current second of the minute as an integer
#define AX_INT_SECOND\
	( AX_StrLitToInt__( __TIME__, 6, 7 ) )

/// \def AX_INT_DATE
/// \brief Integer version of the date in the format of YYMMDD (decimal)
#define AX_INT_DATE\
	( AX_INT_YEAR*10000 + AX_INT_MONTH*100 + AX_INT_DAY )
/// \def AX_INT_TIME
/// \brief Integer version of the time in the format of HHMMSS (decimal)
#define AX_INT_TIME\
	( AX_INT_HOUR*10000 + AX_INT_MINUTE*100 + AX_INT_SECOND )

/// \def AX_INT_DATE_TIME
/// \brief Integer version of the date and time combined in the format of
///        YYMMDDHHMM (decimal)
#define AX_INT_DATE_TIME\
	( AX_INT_DATE*10000 + AX_INT_TIME/100 )
