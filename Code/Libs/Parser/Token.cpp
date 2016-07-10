#include "Token.hpp"
#include "Source.hpp"

namespace Ax { namespace Parser {

	static const char *GetSourceText( const CSource *pSource, uintptr uOffset, uintptr cLength )
	{
		if( !pSource || intptr( cLength ) <= 0 ) {
			return "";
		}

		const char *const pszSourceText = pSource->GetText();
		if( !pszSourceText ) {
			return "";
		}

		return pszSourceText + uOffset;
	}

	TArray< uint8 > SToken::GetEncodedString() const
	{
		if( Type != ETokenType::String || !pSource ) {
			return TArray< uint8 >();
		}

		const TArray< uint8 > &Src = pSource->GetData();
		return TArray< uint8 >( Data.cBytes, Src.Pointer( Data.uOffset ) );
	}
	
	bool SToken::Compare( const String &Other ) const
	{
		if( Other.Len() != cLength ) {
			return false;
		}

		return Other.Compare( GetSourceText( pSource, uOffset, cLength ), 0, cLength );
	}
	bool SToken::CaseCompare( const String &Other ) const
	{
		if( Other.Len() != cLength ) {
			return false;
		}

		return Other.CaseCompare( GetSourceText( pSource, uOffset, cLength ) );
	}

	bool SToken::Compare( const char *pszOther ) const
	{
		if( !Ax::Compare( GetSourceText( pSource, uOffset, cLength ), pszOther, cLength ) ) {
			return false;
		}

		return pszOther[ cLength ] == '\0';
	}
	bool SToken::CaseCompare( const char *pszOther ) const
	{
		if( !Ax::CaseCompare( GetSourceText( pSource, uOffset, cLength ), pszOther, cLength ) ) {
			return false;
		}

		return pszOther[ cLength ] == '\0';
	}

	const char *SToken::GetPointer() const
	{
		if( !pSource || intptr( cLength ) <= 0 ) {
			return nullptr;
		}

		const char *const pszSourceText = pSource->GetText();
		if( !pszSourceText ) {
			return nullptr;
		}

		return pszSourceText + uOffset;
	}

	void SToken::Report( ESeverity Sev, const String &Message ) const
	{
		if( pSource != nullptr ) {
			pSource->Report( Sev, Message, uOffset );
		} else {
			Ax::Report( Sev, nullptr, 0, Message );
		}
	}

}}
