#pragma once

#include <Parser/Source.hpp>

namespace Tenshi { namespace Compiler {

	enum EKeyword : Ax::uint32
	{
		kKeyword_User,

		kKeyword_Null,
		kKeyword_False,
		kKeyword_True,

		kKeyword_Type,
		kKeyword_EndType,

		kKeyword_Function,
		kKeyword_Return,
		kKeyword_EndFunction,

		kKeyword_Select,
		kKeyword_Case,
		kKeyword_CaseDefault,
		kKeyword_Fallthrough,
		kKeyword_EndCase,
		kKeyword_EndSelect,
		kKeyword_Exit,

		kKeyword_If,
		kKeyword_Then,
		kKeyword_Else,
		kKeyword_ElseIf,
		kKeyword_EndIf,

		kKeyword_Do,
		kKeyword_Loop,
		kKeyword_While,
		kKeyword_EndWhile,
		kKeyword_Repeat,
		kKeyword_Until,
		kKeyword_For,
		kKeyword_ForEach,
		kKeyword_To,
		kKeyword_In,
		kKeyword_Step,
		kKeyword_Next,
		kKeyword_ExitLoop,
		kKeyword_RepeatLoop,

		kKeyword_Goto,
		kKeyword_Gosub,
		kKeyword_GoBack,

		kKeyword_Local,
		kKeyword_Global,
		kKeyword_As,

		kKeyword_Enum,
		kKeyword_EndEnum,

		kKeyword_IsAnyOf,
		kKeyword_HasAnyOf,
		kKeyword_HasAllOf,
		kKeyword_HasOnly,

		kKeyword_Inc,
		kKeyword_Dec,
		kKeyword_PreInc,
		kKeyword_PreDec,

		kKeyword_AtomicInc,
		kKeyword_AtomicDec,
		kKeyword_AtomicAdd,
		kKeyword_AtomicSub,
		kKeyword_AtomicAnd,
		kKeyword_AtomicOr,
		kKeyword_AtomicXor,
		kKeyword_AtomicSet,
		kKeyword_AtomicCompareSet,
		kKeyword_AtomicFence,

		kKeyword_JobFunction,
		kKeyword_ThreadFunction,
		kKeyword_ParallelFor,
		kKeyword_ParallelForEach,

		kKeyword_Type_Start__,
			kKeyword_Int8 = kKeyword_Type_Start__,
			kKeyword_Int16,
			kKeyword_Int32,
			kKeyword_Int64,
			kKeyword_Int128,
			kKeyword_IntPtr,
			kKeyword_UInt8,
			kKeyword_UInt16,
			kKeyword_UInt32,
			kKeyword_UInt64,
			kKeyword_UInt128,
			kKeyword_UIntPtr,
			kKeyword_Float32,
			kKeyword_Float64,
			kKeyword_Float32x4,
			kKeyword_Float64x2,
			kKeyword_Int32x4,
			kKeyword_Int64x2,
			kKeyword_Float64x4,
			kKeyword_Int64x4,
			kKeyword_Boolean,
			kKeyword_String,
			kKeyword_Vector2,
			kKeyword_Vector3,
			kKeyword_Vector4,
			kKeyword_Matrix2,
			kKeyword_Matrix3,
			kKeyword_Matrix4,
			kKeyword_Matrix23,
			kKeyword_Matrix24,
			kKeyword_Matrix32,
			kKeyword_Matrix34,
			kKeyword_Matrix42,
			kKeyword_Matrix43,
			kKeyword_Quaternion,
		kKeyword_Type_End__,

		kKeyword_Op_Start__ = kKeyword_Type_End__,
		kKeyword_Op_Not,
		kKeyword_Op_And,
		kKeyword_Op_Or,
		kKeyword_Op_Xor,
		kKeyword_Op_Mod,
		kKeyword_Op_BitwiseLShift,
		kKeyword_Op_BitwiseRShift,
		kKeyword_Op_BitwiseLRotate,
		kKeyword_Op_BitwiseRRotate,
		kKeyword_Op_BitwiseAnd,
		kKeyword_Op_BitwiseOr,
		kKeyword_Op_BitwiseXor,
		kKeyword_Op_BitwiseNot,
		kKeyword_Op_End__,

		kKeyword_RT_End = kKeyword_Op_End__,
		kKeyword_RT_Data,
		kKeyword_RT_Read,
		kKeyword_RT_Restore,

		kKeyword_RT_Dim,
		kKeyword_RT_Redim,
		kKeyword_RT_Undim
	};

#define TENSHI_PARSER_DICTIONARY_ALLOWED\
	"abcdefghijklmnopqrstuvwxyz" \
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
	"_0123456789\xF4\xF5" \
	"#$\? "

	enum
	{
		kSourceFlags =
			// Combine lines separated by the concatenation symbol (default: '\')
			Ax::Parser::kSrcF_CombineLines |
			// Expand string escape sequences
			Ax::Parser::kSrcF_StringEscapes |
			// Support <n>x notations, e.g., 16xA, 8x12, 2x1010
			Ax::Parser::kSrcF_ArbitraryBases |
			// Support Unicode identifiers
			Ax::Parser::kSrcF_UnicodeNames |
			// Support spaces in identifier names (e.g., `make cube` is a single token)
			Ax::Parser::kSrcF_SpacedNames |
			// Support use of a character (default "`") to escape keywords (e.g., `if`)
			Ax::Parser::kSrcF_KeywordEscapes |
			// Use 0c for octal notation instead of 0<number>
			Ax::Parser::kSrcF_BasicOctals |
			// Use % for binary notation in addition to 0b<number>
			Ax::Parser::kSrcF_BasicBinaries |
			// When combining lines (kSrcF_CombineLines) do not do so within comments
			Ax::Parser::kSrcF_NoCommentLineConcat |
			// Do not treat 'x' as a character string
			Ax::Parser::kSrcF_NoCharStrings |
			// Expand string escapes only if an escape prefix is given (default: '~')
			Ax::Parser::kSrcF_StringEscapesPrefixed |
			// Enable REM, REMSTART, and REMEND comments
			Ax::Parser::kSrcF_RemComments |
			// Allow any of the set characters (default: "#$") to be the ending of an identifier
			Ax::Parser::kSrcF_SpecialEndings |
			// Named operators are case insensitive (e.g., "and" is the same as "AND" and "AnD"; all of which are "&&")
			Ax::Parser::kSrcF_CaselessNamedOps |
			// Use $ for hexadecimal notation in addition to 0x<number>
			Ax::Parser::kSrcF_BasicHexadecimals |
			// Do not allow qualifiers on numbers (e.g., 1f)
			Ax::Parser::kSrcF_NoNumberQualifiers
	};

	static const Ax::ECase SourceCasing = Ax::ECase::Insensitive;
	static const char *const pszSourceSpecialEndings = "#$?";

	void SetStringEscapes( Ax::Parser::CSourceProcessor &SrcProc );
	void SetOperators( Ax::Parser::CSourceProcessor &SrcProc );
	void SetNamedOperators( Ax::Parser::CSourceProcessor &SrcProc );
	void SetCoreKeywords( Ax::Parser::CSourceProcessor &SrcProc );

	inline void ConfigureSourceProcessor( Ax::Parser::CSourceProcessor &SrcProc )
	{
		SetStringEscapes( SrcProc );
		SetOperators( SrcProc );
		SetNamedOperators( SrcProc );
		SetCoreKeywords( SrcProc );
		
		SrcProc.FlagsOn( kSourceFlags );
		SrcProc.SetSpecialEndChars( pszSourceSpecialEndings );
		SrcProc.SetDigitSeparator( '_' );
		SrcProc.UseCLineCat();
		SrcProc.SetLineSeparator( ":" );

		SrcProc.AddLineComment( "`" );
		SrcProc.AddLineComment( "//" );

		SrcProc.AddNestComment( "/*", "*/" );
	}

	const Ax::Parser::CSourceProcessor &GetDefaultSourceProcessor();
	void InstallKeyword( const char *pszName, void *pKeywordData, Ax::uint32 uIdentifier = 0, Ax::Parser::EKeywordExists Mode = Ax::Parser::EKeywordExists::Error );

	Ax::Parser::SKeyword *GetEndifKeywordPtr();

}}
