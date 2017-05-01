#include "Config.hpp"
#include "../Core/Assert.hpp"
#include "../Core/TypeTraits.hpp"
#include "../Core/Logger.hpp"

using namespace Ax;

/*
===============================================================================

	PARSING (BASE)

===============================================================================
*/

namespace Parsing
{

	static uintptr SkipSpaces( const char *&p )
	{
		AX_ASSERT_NOT_NULL( p );

		const char *const base = p;

		while( *p <= ' ' && *p != '\n' && *p != '\0' ) {
			++p;
		}

		return p - base;
	}

	static uintptr SkipLine( const char *&p )
	{
		AX_ASSERT_NOT_NULL( p );

		const char *const base = p;

		while( *p != '\n' && *p != '\0' ) {
			++p;
		}

		if( *p == '\n' ) {
			++p;
		}

		return p - base;
	}

#if 0
	static uintptr SkipNonwhite( const char *&p )
	{
		AX_ASSERT_NOT_NULL( p );

		const char *const base = p;

		while( *p > ' ' ) {
			++p;
		}

		return p - base;
	}
	static uintptr SkipQuote( const char *&p )
	{
		AX_ASSERT_NOT_NULL( p );

		if( *p != '\"' ) {
			return 0;
		}

		const char *const base = p;
		
		do {
			++p;
		} while( *p != '\"' && *p != '\0' );

		return p - base;
	}
#endif

	static void CalculateLineInfo( Console::SConfigLineInfo &dst, const char *src, const char *ptr )
	{
		AX_ASSERT_NOT_NULL( src );
		AX_ASSERT_NOT_NULL( ptr );

		const char *p = src;

		dst.line = 1;
		while( p < ptr ) {
			if( *p != '\n' ) {
				++p;
				continue;
			}

			++p;
			++dst.line;
			dst.lineOffset = p - src;
		}

		dst.fileOffset = p - src;
		dst.column = dst.fileOffset - dst.lineOffset + 1;
	}

}

/*
===============================================================================

	CONFIGURATION

===============================================================================
*/

Ax::Console::Configuration::Configuration()
: m_uIncludeDepth( 0 )
{
}
Ax::Console::Configuration::~Configuration()
{
	Clear();
}

void Ax::Console::Configuration::Clear()
{
	while( m_Sections.HeadLink() != nullptr ) {
		RemoveVar( m_Sections.Head() );
	}
}

bool Ax::Console::Configuration::LoadFromMemory( const char *const filename, const char *const buffer )
{
	//
	//	NOTE: Purposely not cleaning up when an error occurs
	//	-     Anything loaded up before the error can still be used, and it's
	//	-     better (for the end user) to at least get something
	//	-     This class cleans up after itself in the destructor
	//

	AX_ASSERT_NOT_NULL( filename );
	AX_ASSERT_NOT_NULL( buffer );

	const char *p = buffer;

	bool isSpecialSection = false;

	SConfigVar *rootSection = nullptr;
	rootSection = FindVar( nullptr, "" );
	if( !rootSection ) {
		rootSection = AddVar( nullptr, "" );
		if( !rootSection ) {
			return false;
		}
	}

	SConfigVar *section = rootSection;

	while( true ) {
		( void )Parsing::SkipSpaces( p );
		if( *p == '\0' ) {
			break;
		}

		// skip comments and blank lines
		if( *p == ';' || *p == '\n' ) {
			( void )Parsing::SkipLine( p );
			continue;
		}

		// start of line
		const char *line_p = p;

		// is this a section?
		if( *p == '[' ) {
			++p;

			const char *s = p;
			const char *e = strchr( s, ']' );

			// make sure there's a matching close bracket
			if( !e ) {
				WarnRaw( filename, buffer, p, "Expected matching ']'" );

				( void )Parsing::SkipLine( p );
				continue;
			}

			// note the location of the first character after that bracket
			const char *next_p = e + 1;

			// make sure there's no newline inside the region
			const char *q = strchr( s, '\n' );
			if( q < e ) {
				WarnRaw( filename, buffer, q, "Unexpected newline" );

				p = q + 1;
				continue;
			}

			// trim leading whitespace
			while( s < e && *s <= ' ' ) {
				++s;
			}

			// trim trailing whitespace
			while( e > s && *( e - 1 ) <= ' ' ) {
				--e;
			}

			// ensure the section name wasn't entirely whitespace
			if( e - s == 0 ) {
				WarnRaw( filename, buffer, p, "Expected section name" );

				( void )Parsing::SkipLine( p );
				continue;
			}

			// save the section
			char sectName[ 256 ];

			if( ( size_t )( e - s ) >= sizeof( sectName ) ) {
				WarnRaw( filename, buffer, s, "Section name will be truncated to 255 characters" );

				e = s + ( sizeof( sectName ) - 1 );
			}

			( void )StrCpyN( sectName, s, e - s );

			// is this the special section? ("Configuration")
			isSpecialSection = false;
			if( CaseCmp( sectName, "Configuration" ) == 0 ) {
				isSpecialSection = true;
			}

			if( isSpecialSection ) {
				section = nullptr;
			} else {
				// check to see if the section exists
				section = FindVar( nullptr, sectName );
				if( !section ) {
					// create the section
					section = AddVar( nullptr, sectName );
					if( !section ) {
						ErrorRaw( filename, buffer, p, "Failed to allocate section" );
						return false;
					}
				}
			}

			// go to the just after the close bracket
			p = next_p;

			// find the next newline
			const char *next_nl = strchr( p, '\n' );
			if( !next_nl ) {
				next_nl = strchr( p, '\0' );
			}

			// ensure that the only characters between here and the next newline
			// are whitespace (or comment)
			while( p < next_nl ) {
				// comment?
				if( *p == ';' ) {
					p = next_nl;
					break;
				}

				if( *p > ' ' ) {
					break;
				}
			}

			if( p < next_nl ) {
				WarnRaw( filename, buffer, p, "Garbage at end of section declaration" );
				p = next_nl;
			}

			// done with this line
			if( *p != '\0' ) {
				++p;
			}
			continue;
		}

		if( !isSpecialSection && section == nullptr ) {
			ErrorRaw( filename, buffer, p, "Invalid internal state" );
			return false;
		}

		// set the default processing mode
		EProcessMode mode = EProcessMode::Set;

		// determine the processing mode if provided
		if( *p == '+' ) {
			++p;
			mode = EProcessMode::Add;
		} else if( *p == '.' ) {
			++p;
			mode = EProcessMode::AddUnique;
		} else if( *p == '-' ) {
			++p;
			mode = EProcessMode::RemoveExact;
		} else if( *p == '!' ) {
			++p;
			mode = EProcessMode::RemoveInexact;
		}

		// find the range of the key
		const char *key_s = p;
		const char *key_e = strchr( key_s, '=' );

		// ensure an equal sign was provided or the mode was "RemoveInexact"
		if( !key_e ) {
			// not inexact mode, so not allowed
			if( mode != EProcessMode::RemoveInexact ) {
				WarnRaw( filename, buffer, p, "Expected '=' after key" );

				( void )Parsing::SkipLine( p );
				continue;
			}

			// find the end of the key
			key_e = strchr( key_s, '\n' );
			if( !key_e ) {
				key_e = strchr( key_s, '\0' );
			}
		}

		// store the start of the next token
		const char *next_p = key_e;

		// trim whitespace
		while( key_e > key_s && *( key_e - 1 ) <= ' ' ) {
			--key_e;
		}

		// check for an invalid value
		if( key_e - key_s == 0 ) {
			WarnRaw( filename, buffer, p, *p == '=' ? "Expected key before '='" : "Expected key" );

			( void )Parsing::SkipLine( p );
			continue;
		}

		// store the name of the key
		char key[ 256 ];
		if( ( size_t )( key_e - key_s ) >= sizeof( key ) ) {
			WarnRaw( filename, buffer, key_s, "Truncating key to 255 characters" );

			key_e = key_s + ( sizeof( key ) - 1 );
		}
		StrCpyN( key, key_s, key_e - key_s );

		// go to the next location
		p = next_p;

		// value buffer
		char value[ 2048 ];
		value[ 0 ] = '\0';

		// read the value
		if( *p == '=' ) {
			++p;

			const char *value_s = p;
			while( *p != '\n' && *p != '\0' ) {
				++p;
			}
			const char *value_e = p;

			// trim spaces
			while( value_e > value_s && *( value_e - 1 ) <= ' ' ) {
				--value_e;
			}
			while( value_s < value_e && *value_s <= ' ' ) {
				++value_s;
			}

			// check if there's room for the value
			if( ( size_t )( value_e - value_s ) >= sizeof( value ) ) {
				WarnRaw( filename, buffer, p, "Value will be truncated to 2047 characters" );

				value_e = value_s + ( sizeof( value ) - 1 );
			}

			// add the value to the buffer
			StrCpyN( value, value_s, value_e - value_s );
		}

		// attempt to process
		Process( section, mode, key, value, filename, buffer, line_p );
	}

	// success!
	return true;
}
bool Ax::Console::Configuration::LoadFromFile( const char *filename )
{
	FILE *fp;

#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
	if( fopen_s( &fp, filename, "r" ) != 0 || !fp ) {
		return false;
	}
#else
	fp = fopen( filename, "r" );
	if( !fp ) {
		return false;
	}
#endif

	fseek( fp, 0, SEEK_END );
	long e = ftell( fp );
	if( e < 0 ) {
		fclose( fp );
		return false;
	}

	fseek( fp, 0, SEEK_SET );

	size_t len = e + 1;
	void *p = malloc( len );
	if( !p ) {
		fclose( fp );
		return false;
	}

#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
	( void )fread_s( p, len, len - 1, 1, fp );
#else
	( void )fread( p, len - 1, 1, fp );
#endif

	fclose( fp );
	fp = nullptr;

	*( ( char * )p + ( len - 1 ) ) = '\0';

	return LoadFromMemory( filename, ( const char * )p );
}

void Ax::Console::Configuration::PrintVars()
{
	for( auto *prnt = m_Sections.Head(); prnt != nullptr; prnt = prnt->siblings.Next() ) {
		PrintVar( *prnt );
	}
}
void Ax::Console::Configuration::PrintVar( const SConfigVar &var )
{
	String buf;

	if( var.pParent != nullptr ) {
		String pre;

		const SConfigVar *prnt = var.pParent->pParent;
		while( prnt != nullptr ) {
			AX_VERIFY( pre.Append( "  " ) == true );
			prnt = prnt->pParent;
		}

		ConfigValueIterator val = var.values.begin();

		if( val != var.values.Last() ) {
			AX_VERIFY( buf.Append( pre ) == true );
			AX_VERIFY( buf.Append( var.name ) == true );
			AX_VERIFY( buf.Append( ":" ) == true );

			while( val != var.values.end() ) {
				if( val->IsEmpty() ) {
					++val;
					continue;
				}

				AX_VERIFY( buf.Append( "\n" ) == true );
				AX_VERIFY( buf.Append( pre ) == true );
				AX_VERIFY( buf.Append( "  " ) == true );
				AX_VERIFY( buf.Append( *val ) == true );

				++val;
			}
		} else if( !!val && !val->IsEmpty() ) {
			AX_VERIFY( buf.Format( "%s%s = %s", pre.CString(), var.name.CString(), val->CString() ) == true );
		} else {
			AX_VERIFY( buf.Format( "%s%s", pre.CString(), var.name.CString() ) == true );
		}
	} else {
		AX_VERIFY( buf.Format( "[%s]", var.name.CString() ) == true );
	}

	printf( "%s\n", buf.CString() );

	for( auto *chld = var.children.Head(); chld != nullptr; chld = chld->siblings.Next() ) {
		PrintVar( *chld );
	}
}

Console::SConfigVar *Ax::Console::Configuration::AddVar( SConfigVar *parent, const char *name )
{
	SConfigVar *var;

#if AX_DEBUG_ENABLED || AX_TEST_ENABLED
	// this should have a unique name
	var = FindVar( parent, name );
	AX_ASSERT( var == nullptr );
#endif

	var = new SConfigVar();
	if( !var ) {
		return nullptr;
	}

	if( !var->name.Assign( name ) ) {
		delete var;
		return nullptr;
	}

	var->siblings.SetNode( var );

	var->pParent = parent;
	if( parent != nullptr ) {
		parent->children.AddTail( var->siblings );
	} else {
		m_Sections.AddTail( var->siblings );
	}

	var->pConfig = this;

	return var;
}
Console::SConfigVar *Ax::Console::Configuration::FindVar( SConfigVar *parent, const char *name )
{
	const TIntrusiveList< SConfigVar > &list = ( parent != nullptr ) ? parent->children : m_Sections;

	for( auto *var = list.Head(); var != nullptr; var = var->siblings.Next() ) {
		if( CaseCmp( var->name, name ) == 0 ) {
			return const_cast< SConfigVar * >( var );
		}
	}

	return nullptr;
}
void Ax::Console::Configuration::RemoveVar( SConfigVar *var )
{
	if( !var ) {
		return;
	}

	while( var->children.HeadLink() != nullptr ) {
		RemoveVar( var->children.Head() );
	}

	TIntrusiveList< SConfigVar > &list = ( var->pParent != nullptr ) ? var->pParent->children : m_Sections;
	list.Unlink( var->siblings );

	var->values.Clear();

	delete var;
}

Console::ConfigValueIterator Ax::Console::Configuration::AddValue( SConfigVar *var, const char *value )
{
	AX_ASSERT_NOT_NULL( var );
	AX_ASSERT_NOT_NULL( value );

	ConfigValueIterator iter = var->values.AddTail();
	AX_VERIFY( iter != var->values.end() );

	AX_VERIFY( iter->Assign( value ) == true );

	return iter;
}
Console::ConfigValueIterator Ax::Console::Configuration::FindValue( SConfigVar *var, const char *value )
{
	AX_ASSERT_NOT_NULL( var );
	AX_ASSERT_NOT_NULL( value );

	for( auto val = var->values.begin(); val != var->values.end(); ++val ) {
		if( *val == value ) {
			return val;
		}
	}

	return var->values.end();
}
void Ax::Console::Configuration::RemoveValue( SConfigVar *var, ConfigValueIterator value )
{
	AX_ASSERT_NOT_NULL( var );
	AX_ASSERT( value != var->values.end() );

	var->values.Remove( value );
}
void Ax::Console::Configuration::RemoveAllValues( SConfigVar *var )
{
	AX_ASSERT_NOT_NULL( var );

	var->values.Clear();
}

#ifdef _WIN32
# define MAXPATH 270
typedef wchar_t CharTy;
typedef DWORD   UIntTy;
#else
# define MAXPATH 512
typedef char   CharTy;
typedef size_t UIntTy;
#endif
static bool get_dir( CharTy *dst, UIntTy dstn ) {
#ifdef _WIN32
	return GetCurrentDirectoryW( dstn, dst ) != 0;
#else
	*dst = CharTy(0);
	return false;
#endif
}
static bool set_dir( const CharTy *src ) {
#ifdef _WIN32
	return SetCurrentDirectoryW( src ) != 0;
#else
	((void)src);
	return false;
#endif
}
static const CharTy *char_strrchr( const CharTy *s, CharTy x ) {
#ifdef _WIN32
	return wcsrchr( s, x );
#else
	return strrchr( s, x );
#endif
}
static void char_strcpyn( CharTy *dst, const CharTy *src, UIntTy dstn ) {
#if defined( _WIN32 ) && defined( _MSC_VER )
	wcsncpy_s( dst, dstn, src, dstn );
#elif defined( _WIN32 )
	wcsncpy( dst, src, dstn );
#else
	strncpy( dst, src, dstn );
#endif
}

void Ax::Console::Configuration::Process( SConfigVar *parent, EProcessMode mode, const char *name, const char *value,
const char *filename, const char *buffer, const char *ptr )
{
	AX_ASSERT_NOT_NULL( name );
	AX_ASSERT_NOT_NULL( value );
	AX_ASSERT_NOT_NULL( buffer );
	AX_ASSERT_NOT_NULL( ptr );

	// special handling
	if( !parent ) {
		if( CaseCmp( name, "BasedOn" ) == 0 ) {
			if( m_uIncludeDepth == 16 ) {
				WarnRaw( filename, buffer, ptr, "Include depth too deep" );
				return;
			}

			CharTy curDir[ MAXPATH ];
			if( !get_dir( &curDir[0], UIntTy(ArraySize(curDir)) ) ) {
				curDir[0] = CharTy(0);
			}

#ifdef _WIN32
			wchar_t filenamebuf[ 512 ] = { L'\0' };
			ToWStr( filenamebuf, filename );
# define filename (&filenamebuf[0])
#endif

			CharTy relDir[ 512 ];
			const CharTy *p = char_strrchr( filename, CharTy('/') );
			const CharTy *q = char_strrchr( filename, CharTy('\\') );
			const CharTy *slash = p < q ? q : p;
			if( slash != nullptr && ( size_t )( slash - filename ) < sizeof( relDir ) ) {
				char_strcpyn( relDir, filename, slash - filename );
				relDir[ slash - filename ] = '\0';

				set_dir( relDir );
			}
#ifdef _WIN32
# undef filename
#endif

			++m_uIncludeDepth;
			const bool r = LoadFromFile( value );
			--m_uIncludeDepth;

			// 'curDir[0]' is set to the null terminator upon failure
			AX_STATIC_SUPPRESS( 6102 ) //'curDir' is initialized but not used
			if( curDir[ 0 ] != UIntTy(0) ) {
				set_dir( curDir );
			}

			if( !r ) {
				String msg;
				msg.Format( "Had trouble loading file: %s", value );

				WarnRaw( filename, buffer, ptr, msg );
			}

			return;
		}

		WarnRaw( filename, buffer, ptr, "Unrecognized command in special section 'Configuration'" );
		return;
	}

	// check if the variable exists
	SConfigVar *var = FindVar( parent, name );
	ConfigValueIterator val;

	// handle the mode
	switch( mode ) {
	case EProcessMode::Set:
		if( !var ) {
			var = AddVar( parent, name );
		}

		RemoveAllValues( var );
		( void )AddValue( var, value );
		break;

	case EProcessMode::Add:
		if( !var ) {
			var = AddVar( parent, name );
		}

		( void )AddValue( var, value );
		break;

	case EProcessMode::AddUnique:
		if( !var ) {
			var = AddVar( parent, name );
		}

		val = FindValue( var, value );
		if( val == var->values.end() ) {
			( void )AddValue( var, value );
		}
		break;

	case EProcessMode::RemoveExact:
		if( !var ) {
			break;
		}

		val = FindValue( var, value );
		if( !val ) {
			break;
		}

		RemoveValue( var, val );
		var = nullptr;
		break;

	case EProcessMode::RemoveInexact:
		if( !var ) {
			break;
		}

		val = FindValue( var, value );
		if( !val ) {
			val = var->values.begin();
			if( val == var->values.end() ) {
				break;
			}
		}

		RemoveValue( var, val );
		var = nullptr;
		break;
	}
}

void Ax::Console::Configuration::Error( const SConfigLineInfo &linfo, const char *message )
{
	AX_ASSERT_NOT_NULL( message );
	Errorf( linfo.filename, ( uint32 )linfo.line, "%s", message );
}
void Ax::Console::Configuration::ErrorRaw( const char *filename, const char *buffer, const char *p, const char *message )
{
	SConfigLineInfo linfo;

	AX_ASSERT_NOT_NULL( filename );
	AX_ASSERT_NOT_NULL( buffer );
	AX_ASSERT_NOT_NULL( p );
	AX_ASSERT_NOT_NULL( message );

	Parsing::CalculateLineInfo( linfo, buffer, p );
	linfo.filename = filename;

	Warn( linfo, message );
}

void Ax::Console::Configuration::Warn( const SConfigLineInfo &linfo, const char *message )
{
	AX_ASSERT_NOT_NULL( message );
	Warnf( linfo.filename, ( uint32 )linfo.line, "%s", message );
}
void Ax::Console::Configuration::WarnRaw( const char *filename, const char *buffer, const char *p, const char *message )
{
	SConfigLineInfo linfo;

	AX_ASSERT_NOT_NULL( filename );
	AX_ASSERT_NOT_NULL( buffer );
	AX_ASSERT_NOT_NULL( p );
	AX_ASSERT_NOT_NULL( message );

	Parsing::CalculateLineInfo( linfo, buffer, p );
	linfo.filename = filename;

	Warn( linfo, message );
}
