#include "Var.hpp"
#include "../Core/Assert.hpp"
#include "../Core/Logger.hpp"
#include "../Math/Const.hpp"
#include "../Math/Basic.hpp"

#define ALLOWEDTEXT "abcdefghijklmnopqrstuvwxyz1234567890_-"
#define SENSITIVITY Ax::ECase::Insensitive

Ax::Console::CVar *Ax::Console::CVar::g_pVIsDevMode = nullptr;
Ax::Console::CVar *Ax::Console::CVar::g_pCSet = nullptr;

namespace Ax { namespace Console {

	static inline bool IsCommandChar( char ch )
	{
		if( ch >= 'a' && ch <= 'z' ) {
			return true;
		}

		if( ch >= 'A' && ch <= 'Z' ) {
			return true;
		}

		if( ch >= '0' && ch <= '9' ) {
			return true;
		}

		if( ch == '_' ) {
			return true;
		}

		return false;
	}
	static inline bool SkipNewlineChars( const char *&s )
	{
		AX_ASSERT_NOT_NULL( s );

		bool found = false;

		if( *s == '\r' ) {
			++s;
			found = true;
		}
		if( *s == '\n' ) {
			++s;
			found = true;
		}

		return found;
	}
	static inline const char *SkipSpace( const char *s )
	{
		AX_ASSERT_NOT_NULL( s );

		while( *s <= ' ' ) {
			if( *s == '\0' ) {
				return s;
			}

			if( *s == '\r' || *s == '\n' ) {
				return s;
			}

			++s;
		}

		return s;
	}
	static inline const char *SkipLine( const char *s )
	{
		AX_ASSERT_NOT_NULL( s );

		while( *s != '\0' ) {
			if( SkipNewlineChars( s ) ) {
				break;
			}

			++s;
		}

		return s;
	}
	static inline bool IsComment( const char *s )
	{
		AX_ASSERT_NOT_NULL( s );

		// comments can begin with ';', '--', or '//'
		if( *s == ';' ) {
			return true;
		}

		if( ( s[ 0 ] == '-' || s[ 0 ] == '/' ) && s[ 1 ] == s[ 0 ] ) {
			return true;
		}

		return false;
	}
	static inline bool HandleSpaceAndComments( const char *&s )
	{
		AX_ASSERT_NOT_NULL( s );

		s = SkipSpace( s );

		if( SkipNewlineChars( s ) ) {
			return true;
		}

		if( IsComment( s ) ) {
			s = SkipLine( s );
			return true;
		}

		return false;
	}

	static int32 DefaultCVarProcessor_f( CVar &Var, uintcpu cArgs, const char *const *ppszArgs )
	{
		// varName value -- set the variable's value
		if( cArgs == 1 ) {
			AX_ASSERT_NOT_NULL( ppszArgs );

			// if variable is const then deny
			if( Var.IsConst() ) {
				// TODO: "Variable is constant"
				return 0;
			}

			// if variable is a cheat and cheating isn't enabled then deny
			if( CVar::g_pVIsDevMode != NULL && CVar::g_pVIsDevMode->GetInteger() != 0 && Var.IsCheat() ) {
				// TODO: Variable is cheat
				return 0;
			}

			// no rejections; set the value
			Var.SetString( ppszArgs[ 0 ] );
			return 1;
		}

		// only other option is retrieving this variable's value
		if( !cArgs ) {
			// TODO: "Unexpected argument count"
			return 0;
		}

		// done
		return 1;
	}
	static int32 Cmd_Set_f( CVar &, uintcpu cArgs, const char *const *ppszArgs )
	{
		//
		//	NOTE: These assertions are just to ensure the calling system is
		//	-     doing its job properly.
		//

		( void )cArgs;
		( void )ppszArgs;

		AX_ASSERT_MSG( cArgs == 2, "'set' takes 2 arguments" );
		AX_ASSERT_NOT_NULL( ppszArgs );

		CVar *pVar = CVar::Find( ppszArgs[ 0 ] );
		if( !pVar ) {
			pVar = New< CVar >();
			if( !pVar ) {
				AX_ERROR_LOG += "Failed to allocate console var";
				return 0;
			}

			if( !pVar->Init( ppszArgs[ 0 ], kVar_User_Bit ) ) {
				AX_ERROR_LOG += "Failed to initialize console var";
				Delete< CVar >( pVar );
				return 0;
			}
		} else if( !pVar->CanModify() ) {
			return 0;
		}

		return +pVar->SetString( ppszArgs[ 1 ] );
	}

	CVar *GetDevModeCVar()
	{
		return CVar::g_pVIsDevMode;
	}
	CVar *FindCVar( const char *name )
	{
		return CVar::Find( name );
	}

	CVar *NewCVar( const char *name, int sysFlags, int memtag )
	{
		CVar *var = New< CVar >( memtag );
		AX_VERIFY_NOT_NULL( var );

		AX_VERIFY( var->Init( name, sysFlags ) == true );

		return var;
	}
	CVar *NewCommandCVar( const char *name, FnVarProcessor func, int memtag )
	{
		CVar *var = New< CVar >( memtag );
		AX_VERIFY_NOT_NULL( var );

		AX_VERIFY( var->InitCommand( name, func ) == true );

		return var;
	}
	CVar *DeleteCVar( CVar *var )
	{
		Delete< CVar >( var );
		return nullptr;
	}

}}

Ax::Console::CVar::State &Ax::Console::CVar::State::Get()
{
	static State instance;
	return instance;
}

Ax::Console::CVar::State::State()
: List()
, Dictionary()
{
}
Ax::Console::CVar::State::~State()
{
}

void Ax::Console::CVar::InitSystem()
{
	// initialize the primary dictionary
	if( !G().Dictionary.IsInitialized() ) {
		if( !AX_VERIFY( G().Dictionary.Init( ALLOWEDTEXT, SENSITIVITY ) == true ) ) { return; }
	}

	// create the "set" command
	if( !g_pCSet ) {
		g_pCSet = New< CVar >();
		if( !AX_VERIFY_NOT_NULL( g_pCSet ) ) { return; }

		if( !AX_VERIFY( g_pCSet->InitCommand( "set", &Cmd_Set_f ) == true ) ) {
			Delete< CVar >( g_pCSet );
			return;
		}

		g_pCSet->SetArgumentsLimit( 2, 2 );
		g_pCSet->m_uSysFlags |= kVar_MinArgs_Bit | kVar_MaxArgs_Bit;
	}

	// create the "devmode" var
	if( !g_pVIsDevMode ) {
		g_pVIsDevMode = New< CVar >();
		if( !AX_VERIFY_NOT_NULL( g_pVIsDevMode ) ) { return; }
		
		if( !AX_VERIFY( g_pVIsDevMode->Init( "devmode", kVar_Init_Bit ) == true ) ) {
			Delete< CVar >( g_pVIsDevMode );
			return;
		}

#if AX_DEBUG_ENABLED
		g_pVIsDevMode->SetString( "1" );
#endif
	}
}
Ax::Console::CVar *Ax::Console::CVar::Find( const char *name )
{
	InitSystem();

	auto *const pEntry = G().Dictionary.Find( name );
	return ( pEntry != nullptr ) ? pEntry->pData : nullptr;
}

Ax::Console::CVar::CVar()
: m_Value()
, m_iValue( 0 )
, m_fValue( 0 )
, m_fMinValue( -AX_INFINITY )
, m_fMaxValue( AX_INFINITY )
, m_uSysFlags( 0 )
, m_uUsrFlags( 0 )
, m_cMinParms( 0 )
, m_cMaxParms( 0 )
, m_pfnProcessor( &DefaultCVarProcessor_f )
, m_pEntry( nullptr )
, m_Link( this )
{
	G().List.AddTail( m_Link );
}
Ax::Console::CVar::~CVar()
{
	Fini();
}

bool Ax::Console::CVar::Init( const char *name, uint32 sysFlags )
{
	InitSystem();

	if( m_pEntry != nullptr ) {
		return true;
	}

	if( !name || *name == '\0' ) {
		AX_ERROR_LOG += "CVar name needed";
		return false;
	}

	auto *pEntry = G().Dictionary.Lookup( name );
	if( !pEntry ) {
		AX_ERROR_LOG += "CVar name format is invalid";
		return false;
	}

	if( pEntry->pData != nullptr ) {
		AX_ERROR_LOG += "CVar already exists";
		return false;
	}

	pEntry->pData = this;
	m_uSysFlags |= sysFlags | kVar_MinArgs_Bit | kVar_MaxArgs_Bit;

	m_cMinParms = 0;
	m_cMaxParms = 1;

	m_pEntry = pEntry;

	return true;
}
bool Ax::Console::CVar::InitCommand( const char *name, FnVarProcessor func )
{
	AX_ASSERT_NOT_NULL( func );

	if( !Init( name, kVar_Const_Bit ) ) {
		return false;
	}

	m_uSysFlags &= ~( kVar_MinArgs_Bit | kVar_MaxArgs_Bit );

	m_cMinParms = 0;
	m_cMaxParms = 0;

	m_pfnProcessor = func;

	return true;
}
void Ax::Console::CVar::Fini()
{
	if( !m_pEntry ) {
		return;
	}

	m_pEntry->pData = nullptr;
}

bool Ax::Console::CVar::SetString( const char *newString )
{
	if( !m_Value.Assign( newString ) ) {
		return false;
	}

	m_uSysFlags |= kVar_Modified_Bit;
	return true;
}
bool Ax::Console::CVar::SetInteger( int newInteger )
{
	if( ( double )newInteger < m_fMinValue ) {
		newInteger = ( int )m_fMinValue;
	} else if( ( double )newInteger > m_fMaxValue ) {
		newInteger = ( int )m_fMaxValue;
	}

	if( !m_Value.Format( "%i", newInteger ) ) {
		return false;
	}

	m_iValue = newInteger;
	m_fValue = ( float )newInteger;

	return true;
}
bool Ax::Console::CVar::SetFloat( float newFloat )
{
	if( ( double )newFloat < m_fMinValue ) {
		newFloat = ( float )m_fMinValue;
	} else if( ( double )newFloat > m_fMaxValue ) {
		newFloat = ( float )m_fMaxValue;
	}

	bool r = false;
	if( newFloat == AX_INFINITY ) {
		r = m_Value.Assign( "#inf" );
	} else if( newFloat == AX_NAN ) {
		r = m_Value.Assign( "#nan" );
	} else if( newFloat == -AX_INFINITY ) {
		r = m_Value.Assign( "-#inf" );
	} else if( newFloat == -AX_NAN ) {
		r = m_Value.Assign( "-#nan" );
	} else {
		r = m_Value.Format( "%g", newFloat );
	}

	if( !r ) {
		return false;
	}

	m_iValue = ( int )newFloat;
	m_fValue = newFloat;

	return true;
}

const char *Ax::Console::CVar::GetString() const
{
	return m_Value.c_str();
}
int Ax::Console::CVar::GetInteger() const
{
	CalculateNumberValues();
	return m_iValue;
}
float Ax::Console::CVar::GetFloat() const
{
	CalculateNumberValues();
	return m_fValue;
}

void Ax::Console::CVar::SetRange( double minRange, double maxRange )
{
	m_fMinValue = minRange;
	m_fMaxValue = maxRange;
}
double Ax::Console::CVar::GetMinimumValue() const
{
	return m_fMinValue;
}
double Ax::Console::CVar::GetMaximumValue() const
{
	return m_fMaxValue;
}

int Ax::Console::CVar::GetSystemFlags() const
{
	return m_uSysFlags;
}
int Ax::Console::CVar::GetUserFlags() const
{
	return m_uUsrFlags;
}

bool Ax::Console::CVar::HasAnySystemFlags( uint32 flags ) const
{
	return !!( m_uSysFlags & flags );
}
bool Ax::Console::CVar::HasAllSystemFlags( uint32 flags ) const
{
	return ( m_uSysFlags & flags ) == flags;
}

bool Ax::Console::CVar::IsUser() const
{
	return !!( m_uSysFlags & kVar_User_Bit );
}
bool Ax::Console::CVar::IsCheat() const
{
	return !!( m_uSysFlags & kVar_Cheat_Bit );
}
bool Ax::Console::CVar::IsServer() const
{
	return !!( m_uSysFlags & kVar_Server_Bit );
}
bool Ax::Console::CVar::IsClient() const
{
	return !!( m_uSysFlags & kVar_Client_Bit );
}
bool Ax::Console::CVar::IsInit() const
{
	return !!( m_uSysFlags & kVar_Init_Bit );
}
bool Ax::Console::CVar::IsConst() const
{
	return !!( m_uSysFlags & kVar_Const_Bit );
}
bool Ax::Console::CVar::IsArchive() const
{
	return !!( m_uSysFlags & kVar_Archive_Bit );
}
bool Ax::Console::CVar::IsUsingMinArgs() const
{
	return !!( m_uSysFlags & kVar_MinArgs_Bit );
}
bool Ax::Console::CVar::IsUsingMaxArgs() const
{
	return !!( m_uSysFlags & kVar_MaxArgs_Bit );
}
bool Ax::Console::CVar::IsModified() const
{
	return !!( m_uSysFlags & kVar_Modified_Bit );
}

bool Ax::Console::CVar::CanModify( EVarPhase Phase ) const
{
	return !( IsConst() | ( IsInit() ^ ( Phase == EVarPhase::Initialization ) ) );
}

void Ax::Console::CVar::SetUserFlags( uint32 flags )
{
	m_uUsrFlags = flags;
}
void Ax::Console::CVar::AddUserFlags( uint32 flags )
{
	m_uUsrFlags |= flags;
}
void Ax::Console::CVar::RemoveUserFlags( uint32 flags )
{
	m_uUsrFlags &= ~flags;
}
bool Ax::Console::CVar::HasAnyUserFlags( uint32 flags ) const
{
	return ( m_uUsrFlags & flags ) != 0;
}
bool Ax::Console::CVar::HasAllUserFlags( uint32 flags ) const
{
	return ( m_uUsrFlags & flags ) == flags;
}

void Ax::Console::CVar::SetArgumentsLimit( uintcpu minArgs, uintcpu maxArgs )
{
	m_cMinParms = minArgs;
	m_cMaxParms = maxArgs;
}
int Ax::Console::CVar::GetMinimumArguments() const
{
	return m_cMinParms;
}
int Ax::Console::CVar::GetMaximumArguments() const
{
	return m_cMaxParms;
}

void Ax::Console::CVar::SetCommandProcessor( FnVarProcessor func )
{
	m_pfnProcessor = func;
}
Ax::Console::FnVarProcessor Ax::Console::CVar::GetCommandProcessor() const
{
	return m_pfnProcessor;
}

bool Ax::Console::CVar::InvokeCommand( uintcpu numArgs, const char *const *args )
{
	if( ( m_uSysFlags & kVar_MinArgs_Bit ) && numArgs < m_cMinParms ) {
		AX_ERROR_LOG += "Too few args for command";
		return false;
	}
	if( ( m_uSysFlags & kVar_MaxArgs_Bit ) && numArgs > m_cMaxParms ) {
		AX_ERROR_LOG += "Too many args for command";
		return false;
	}

	AX_ASSERT_NOT_NULL( m_pfnProcessor );
	if( !m_pfnProcessor ) {
		return false;
	}

	return !!m_pfnProcessor( *this, numArgs, args );
}
//
//	TODO: Refactor this
//
bool Ax::Console::CVar::ExecuteCommands( const char *source, const char *filename, int baseLine )
{
	char memBuf[ 8192 + 2048 ];
	char *args[ 128 ];

	const size_t memBufSize = sizeof( memBuf );
	const size_t maxArgs = ArraySize( args );

	AX_ASSERT_NOT_NULL( source );
	AX_ASSERT_NOT_NULL( filename );
	AX_ASSERT_MSG( baseLine > 0, "Invalid starting line" );

	int currLine = baseLine;
	const char *p = source;

	bool lastResult = true;

	// main parse loop
	while( *p != '\0' ) {
		// skip spaces and comments
		if( HandleSpaceAndComments( p ) ) {
			++currLine;
			continue;
		}

		const unsigned int funcLine = currLine;

		// mark the first command
		const char *cmdS = p;
		while( IsCommandChar( *p ) ) {
			++p;
		}
		const char *cmdE = p;

		if( ( size_t )( cmdE - cmdS ) >= memBufSize ) {
			Errorf( filename, currLine, "Command name is too long" );
			p = SkipLine( p );
			++currLine;
			lastResult = false;
			continue;
		}

		memcpy( &memBuf[ 0 ], cmdS, ( size_t )( cmdE - cmdS ) );
		memBuf[ ( size_t )( cmdE - cmdS ) ] = '\0';

		CVar *var = CVar::Find( memBuf );
		if( !var ) {
			Errorf( filename, currLine, "Variable does not exist" );
			p = SkipLine( p );
			++currLine;
			lastResult = false;
			continue;
		}

		size_t memBufIndex = 0;
		unsigned int numArgs = 0;

		// handle each parameter
		while( *p != '\0' ) {
			if( HandleSpaceAndComments( p ) ) {
				++currLine;
				break;
			}

			if( numArgs == maxArgs ) {
				Errorf( filename, currLine, "Too many arguments" );
				p = SkipLine( p );
				++currLine;
				var = nullptr;
				lastResult = false;
				break;
			}

			const char *s = p;
			const char *e = p;
			if( *s == '\"' ) {
				++s;

				while( *p != '\"' && *p != '\0' ) {
					if( *p == '\\' ) {
						++p;

						if( *p == '\0' ) {
							break;
						}
					}

					++p;
				}

				e = p;
			} else {
				while( *p > ' ' ) {
					++p;
				}

				e = p;
			}

			if( memBufIndex + ( size_t )( e - s ) > memBufSize ) {
				Errorf( filename, currLine, "Line is too long" );
				lastResult = false;
				break;
			}

			args[ numArgs++ ] = &memBuf[ memBufIndex ];
			memcpy( &memBuf[ memBufIndex ], s, ( size_t )( e - s ) );
			memBufIndex += ( size_t )( e - s );
			memBuf[ memBufIndex++ ] = '\0';
		}

		// invoke the command; var can be nullptr as hack for loop exit
		if( var != nullptr ) {
			if( !var->InvokeCommand( numArgs, args ) ) {
				Errorf( filename, funcLine, "Command invocation failed" );
				lastResult = false;
			} else {
				lastResult = true;
			}
		}
	}

	return lastResult;
}

void Ax::Console::CVar::CalculateNumberValuesNoRange() const
{
	if( ~m_uSysFlags & kVar_Modified_Bit ) {
		return;
	}

	m_uSysFlags &= ~kVar_Modified_Bit;

	m_iValue = atoi( m_Value.c_str() );
	m_fValue = ( float )atof( m_Value.c_str() );

	if( m_Value.CaseCmp( "true" ) || m_Value.CaseCmp( "on" ) || m_Value.CaseCmp( "yes" ) || m_Value.CaseCmp( "enabled" ) ) {
		m_iValue = 1;
		m_fValue = 1;
		return;
	}

	const char *p = m_Value.c_str();
	float sign = 1;
	if( *p == '-' ) {
		sign = -1;
		++p;
	}

	if( CaseCmp( p, "#inf" ) ) {
		m_iValue = -1;
		m_fValue = sign*AX_INFINITY;
		return;
	}

	if( CaseCmp( p, "#nan" ) ) {
		m_iValue = -1;
		m_fValue = sign*AX_NAN;
		return;
	}
}
void Ax::Console::CVar::CalculateNumberValues() const
{
	CalculateNumberValuesNoRange();

	const double x = ( double )m_fValue;
	if( x < m_fMinValue ) {
		m_iValue = ( int )m_fMinValue;
		m_fValue = ( float )m_fMinValue;
	} else if( x > m_fMaxValue ) {
		m_iValue = ( int )m_fMaxValue;
		m_fValue = ( float )m_fMaxValue;
	}
}
