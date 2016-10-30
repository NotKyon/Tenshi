#include "_PCH.hpp"
#include "ParserConfig.hpp"

namespace Tenshi { namespace Compiler {

	Ax::Parser::CSourceProcessor &GetDefaultSourceProcessorMutable()
	{
		static Ax::Parser::CSourceProcessor Instance( TENSHI_PARSER_DICTIONARY_ALLOWED, SourceCasing );
		static bool bDidConfigure = false;

		if( !bDidConfigure ) {
			bDidConfigure = true;
			ConfigureSourceProcessor( Instance );
		}

		return Instance;
	}
	const Ax::Parser::CSourceProcessor &GetDefaultSourceProcessor()
	{
		return GetDefaultSourceProcessorMutable();
	}
	void InstallKeyword( const char *pszName, void *pKeywordData, Ax::uint32 uIdentifier, Ax::Parser::EKeywordExists Mode )
	{
		GetDefaultSourceProcessorMutable().AddKeyword( Ax::Parser::SKeyword( pszName, uIdentifier, pKeywordData ), Mode );
	}

	void SetStringEscapes( Ax::Parser::CSourceProcessor &SrcProc )
	{
		SrcProc.ResetEscapes();

		SrcProc.SetStringEscapeChar( '\\', '~' );

		SrcProc.SetStringEscape( '0', '\0' );
		SrcProc.SetStringEscape( 'a', '\a' );
		SrcProc.SetStringEscape( 'b', '\b' );
		SrcProc.SetStringEscape( 'f', '\f' );
		SrcProc.SetStringEscape( 'n', '\n' );
		SrcProc.SetStringEscape( 'q', '\"' );
		SrcProc.SetStringEscape( 'r', '\r' );
		SrcProc.SetStringEscape( 't', '\t' );
		SrcProc.SetStringEscape( 'v', '\v' );
		SrcProc.SetStringEscape( 'A', '\a' );
		SrcProc.SetStringEscape( 'B', '\b' );
		SrcProc.SetStringEscape( 'F', '\f' );
		SrcProc.SetStringEscape( 'N', '\n' );
		SrcProc.SetStringEscape( 'Q', '\"' );
		SrcProc.SetStringEscape( 'R', '\r' );
		SrcProc.SetStringEscape( 'T', '\t' );
		SrcProc.SetStringEscape( 'V', '\v' );
		SrcProc.SetStringEscape( '\'', '\'' );
		SrcProc.SetStringEscape( '\"', '\"' );
		SrcProc.SetStringEscape( '\?', '\?' );
		SrcProc.SetStringEscape( '\\', '\\' );

		SrcProc.SetStringEscapeMode( 'x', Ax::Parser::EStringEscape::InsertByteValue );
		SrcProc.SetStringEscapeMode( 'X', Ax::Parser::EStringEscape::InsertByteValue );
		SrcProc.SetStringEscapeMode( 'u', Ax::Parser::EStringEscape::InsertUnicode );
		SrcProc.SetStringEscapeMode( 'U', Ax::Parser::EStringEscape::InsertUnicode );
	}
	void SetOperators( Ax::Parser::CSourceProcessor &SrcProc )
	{
		static const char *const pszOperators[] = {
			"~<<=",	// Assignment bitwise left-rotate (v ~<<= r)
			"~>>=",	// Assignment bitwise right-rotate (v ~>>= r)
			"<<=",	// Assignment bitwise left-shift (v <<= r)
			">>=",	// Assignment bitwise right-shift (v >>= r)
			"~<<",	// Bitwise left-rotate (l ~<< r)
			"~>>",	// Bitwise right-rotate (l ~>> r)
			"%%=",	// Assignment modulus (v %%= r)
			"<>",	// Not-equal comparison (l <> r) :: same as l != r
			"!=",	// Not-equal comparison (l != r) :: same as l <> r
			"<=",	// Less-equal comparison (l <= r)
			">=",	// Greater-equal comparison (l >= r)
			":=",	// Clarified assignment (v := r)
			"==",	// Clarified equivalence comparison (v == r)
			"+=",	// Assignment add (v += r)
			"-=",	// Assignment subtract (v -= r)
			"*=",	// Assignment multiply (v *= r)
			"/=",	// Assignment divide (v /= r)
			"^=",	// Assignment power (v ^= r)
			"<<",	// Bitwise left-shift (l << r)
			">>",	// Bitwise right-shift (l >> r)
			"~~",	// Logical exclusive-or (IF l ~~ r)
			"||",	// Logical inclusive-or (IF l || r)
			"&&",	// Logical and (IF l && r)
			"%%",	// Modulus (l %% r)
			"&=",	// Assignment bitwise and (v &= r)
			"|=",	// Assignment bitwise or (v |= r)
			"~=",	// Assignment bitwise xor (v ~= r)
			"=>",	// Function short-hand
			"+",	// Add (l + r)
			"-",	// Subtract (l - r); Negate (-r)
			"*",	// Multiply (l * r); Pointer indirection (*r)
			"/",	// Divide (l / r); Append path (l$ / r$)
			"^",	// Power (l ^ r)
			"=",	// Assignment (v = r); comparison (IF v = r...)
			"!",	// Logical not (!r)
			"<",	// Less-than comparison (l < r)
			">",	// Greater-than comparison (l > r)
			"~",	// Bitwise exclusive-or (l ~ r); Bitwise complement (~r)
			"&",	// Bitwise and (l & r); Address-of (&r)
			"|",	// Bitwise or (l | r)
			"(",	// Left parenthesis (open group; function call; array subscript)
			")",	// Right parenthesis (close group; function call; array subscript)
			"[",	// Left square bracket (open group; explicit array subscript)
			"]",	// Right square bracket (close group; explicit array subscript)
			"@",	// Attribute (at-sign)
			",",	// Parameter separator (comma)
			";",	// Statement parameter separator (semicolon)
			".",	// Field accessor (period)
			":",	// Statement separator (colon)
			"#"		// Preprocessor directive
		};

		SrcProc.AddOperators( pszOperators );
	}
	void SetNamedOperators( Ax::Parser::CSourceProcessor &SrcProc )
	{
#if 0
		SrcProc.AddNamedOperator( "NOT", "!" );
		SrcProc.AddNamedOperator( "AND", "&&" );
		SrcProc.AddNamedOperator( "OR", "||" );
		SrcProc.AddNamedOperator( "XOR", "~~" );
		SrcProc.AddNamedOperator( "MOD", "%%" );
		SrcProc.AddNamedOperator( "BITWISE LSHIFT", "<<" );
		SrcProc.AddNamedOperator( "BITWISE RSHIFT", ">>" );
		SrcProc.AddNamedOperator( "BITWISE LROTATE", "~<<" );
		SrcProc.AddNamedOperator( "BITWISE RROTATE", "~>>" );
		SrcProc.AddNamedOperator( "BITWISE AND", "&" );
		SrcProc.AddNamedOperator( "BITWISE OR", "|" );
		SrcProc.AddNamedOperator( "BITWISE XOR", "~" );
		SrcProc.AddNamedOperator( "BITWISE NOT", "~" );
#endif
	}
	void SetCoreKeywords( Ax::Parser::CSourceProcessor &SrcProc )
	{
		static const Ax::Parser::SKeyword Keywords[] = {
			Ax::Parser::SKeyword( "NULL",				kKeyword_Null ),
			Ax::Parser::SKeyword( "FALSE",				kKeyword_False ),
			Ax::Parser::SKeyword( "TRUE",				kKeyword_True ),

			Ax::Parser::SKeyword( "TYPE",				kKeyword_Type ),
			Ax::Parser::SKeyword( "ENDTYPE",			kKeyword_EndType ),

			Ax::Parser::SKeyword( "FUNCTION",			kKeyword_Function ),
			Ax::Parser::SKeyword( "EXITFUNCTION",		kKeyword_ExitFunction ),
			Ax::Parser::SKeyword( "ENDFUNCTION",		kKeyword_EndFunction ),

			Ax::Parser::SKeyword( "SELECT",				kKeyword_Select ),
			Ax::Parser::SKeyword( "CASE",				kKeyword_Case ),
			Ax::Parser::SKeyword( "CASE DEFAULT",		kKeyword_CaseDefault ),
			Ax::Parser::SKeyword( "FALLTHROUGH",		kKeyword_Fallthrough ),
			Ax::Parser::SKeyword( "ENDCASE",			kKeyword_EndCase ),
			Ax::Parser::SKeyword( "ENDSELECT",			kKeyword_EndSelect ),
			Ax::Parser::SKeyword( "EXIT",				kKeyword_Exit ),

			Ax::Parser::SKeyword( "IF",					kKeyword_If ),
			Ax::Parser::SKeyword( "THEN",				kKeyword_Then ),
			Ax::Parser::SKeyword( "ELSE",				kKeyword_Else ),
			Ax::Parser::SKeyword( "ELSEIF",				kKeyword_ElseIf ),
			Ax::Parser::SKeyword( "ENDIF",				kKeyword_EndIf ),

			Ax::Parser::SKeyword( "DO",					kKeyword_Do ),
			Ax::Parser::SKeyword( "LOOP",				kKeyword_Loop ),
			Ax::Parser::SKeyword( "WHILE",				kKeyword_While ),
			Ax::Parser::SKeyword( "ENDWHILE",			kKeyword_EndWhile ),
			Ax::Parser::SKeyword( "REPEAT",				kKeyword_Repeat ),
			Ax::Parser::SKeyword( "UNTIL",				kKeyword_Until ),
			Ax::Parser::SKeyword( "FOR",				kKeyword_For ),
			Ax::Parser::SKeyword( "FOREACH",			kKeyword_ForEach ),
			Ax::Parser::SKeyword( "TO",					kKeyword_To ),
			Ax::Parser::SKeyword( "IN",					kKeyword_In ),
			Ax::Parser::SKeyword( "STEP",				kKeyword_Step ),
			Ax::Parser::SKeyword( "NEXT",				kKeyword_Next ),
			Ax::Parser::SKeyword( "EXIT LOOP",			kKeyword_ExitLoop ),
			Ax::Parser::SKeyword( "REPEAT LOOP",        kKeyword_RepeatLoop ),

			Ax::Parser::SKeyword( "GOTO",				kKeyword_Goto ),
			Ax::Parser::SKeyword( "GOSUB",				kKeyword_Gosub ),
			Ax::Parser::SKeyword( "RETURN",				kKeyword_Return ),

			Ax::Parser::SKeyword( "LOCAL",				kKeyword_Local ),
			Ax::Parser::SKeyword( "GLOBAL",				kKeyword_Global ),
			Ax::Parser::SKeyword( "AS",					kKeyword_As ),

			Ax::Parser::SKeyword( "ENUM",				kKeyword_Enum ),
			Ax::Parser::SKeyword( "ENDENUM",			kKeyword_EndEnum ),

			Ax::Parser::SKeyword( "IS ANY OF",			kKeyword_IsAnyOf ),
			Ax::Parser::SKeyword( "HAS ANY OF",			kKeyword_HasAnyOf ),
			Ax::Parser::SKeyword( "HAS ALL OF",			kKeyword_HasAllOf ),
			Ax::Parser::SKeyword( "HAS ONLY",			kKeyword_HasOnly ),

			Ax::Parser::SKeyword( "INC",				kKeyword_Inc ),
			Ax::Parser::SKeyword( "DEC",				kKeyword_Dec ),
			Ax::Parser::SKeyword( "PRE INC",			kKeyword_PreInc ),
			Ax::Parser::SKeyword( "PRE DEC",			kKeyword_PreDec ),

			Ax::Parser::SKeyword( "ATOMIC INC",			kKeyword_AtomicInc ),
			Ax::Parser::SKeyword( "ATOMIC DEC",			kKeyword_AtomicDec ),
			Ax::Parser::SKeyword( "ATOMIC ADD",			kKeyword_AtomicAdd ),
			Ax::Parser::SKeyword( "ATOMIC SUB",			kKeyword_AtomicSub ),
			Ax::Parser::SKeyword( "ATOMIC AND",			kKeyword_AtomicAnd ),
			Ax::Parser::SKeyword( "ATOMIC OR",			kKeyword_AtomicOr ),
			Ax::Parser::SKeyword( "ATOMIC XOR",			kKeyword_AtomicXor ),
			Ax::Parser::SKeyword( "ATOMIC SET",			kKeyword_AtomicSet ),
			Ax::Parser::SKeyword( "ATOMIC COMPARE SET",	kKeyword_AtomicCompareSet ),
			Ax::Parser::SKeyword( "ATOMIC FENCE",		kKeyword_AtomicFence ),

			Ax::Parser::SKeyword( "JOB FUNCTION",		kKeyword_JobFunction ),
			Ax::Parser::SKeyword( "THREAD FUNCTION",	kKeyword_ThreadFunction ),
			Ax::Parser::SKeyword( "PARALLEL FOR",		kKeyword_ParallelFor ),
			Ax::Parser::SKeyword( "PARALLEL FOREACH",	kKeyword_ParallelForEach ),

			Ax::Parser::SKeyword( "INT8",				kKeyword_Int8 ),
			Ax::Parser::SKeyword( "INT16",				kKeyword_Int16 ),
			Ax::Parser::SKeyword( "INT32",				kKeyword_Int32 ),
			Ax::Parser::SKeyword( "INT64",				kKeyword_Int64 ),
			Ax::Parser::SKeyword( "INT128",				kKeyword_Int128 ),
			Ax::Parser::SKeyword( "INTPTR",				kKeyword_IntPtr ),
			Ax::Parser::SKeyword( "UINT8",				kKeyword_UInt8 ),
			Ax::Parser::SKeyword( "UINT16",				kKeyword_UInt16 ),
			Ax::Parser::SKeyword( "UINT32",				kKeyword_UInt32 ),
			Ax::Parser::SKeyword( "UINT64",				kKeyword_UInt64 ),
			Ax::Parser::SKeyword( "UINT128",			kKeyword_UInt128 ),
			Ax::Parser::SKeyword( "UINTPTR",			kKeyword_UIntPtr ),
			Ax::Parser::SKeyword( "FLOAT32",			kKeyword_Float32 ),
			Ax::Parser::SKeyword( "FLOAT64",			kKeyword_Float64 ),
			Ax::Parser::SKeyword( "FLOAT32X4",			kKeyword_Float32x4 ),
			Ax::Parser::SKeyword( "FLOAT64X2",			kKeyword_Float64x2 ),
			Ax::Parser::SKeyword( "INT32X4",			kKeyword_Int32x4 ),
			Ax::Parser::SKeyword( "INT64X2",			kKeyword_Int64x2 ),
			Ax::Parser::SKeyword( "FLOAT64X4",			kKeyword_Float64x4 ),
			Ax::Parser::SKeyword( "INT64X4",			kKeyword_Int64x4 ),
			Ax::Parser::SKeyword( "BOOLEAN",			kKeyword_Boolean ),
			Ax::Parser::SKeyword( "STRING",				kKeyword_String ),
			Ax::Parser::SKeyword( "VECTOR2",			kKeyword_Vector2 ),
			Ax::Parser::SKeyword( "VECTOR3",			kKeyword_Vector3 ),
			Ax::Parser::SKeyword( "VECTOR4",			kKeyword_Vector4 ),
			Ax::Parser::SKeyword( "MATRIX2",			kKeyword_Matrix2 ),
			Ax::Parser::SKeyword( "MATRIX3",			kKeyword_Matrix3 ),
			Ax::Parser::SKeyword( "MATRIX4",			kKeyword_Matrix4 ),
			Ax::Parser::SKeyword( "MATRIX23",			kKeyword_Matrix23 ),
			Ax::Parser::SKeyword( "MATRIX24",			kKeyword_Matrix24 ),
			Ax::Parser::SKeyword( "MATRIX32",			kKeyword_Matrix32 ),
			Ax::Parser::SKeyword( "MATRIX34",			kKeyword_Matrix34 ),
			Ax::Parser::SKeyword( "MATRIX42",			kKeyword_Matrix42 ),
			Ax::Parser::SKeyword( "MATRIX43",			kKeyword_Matrix43 ),
			Ax::Parser::SKeyword( "QUATERNION",			kKeyword_Quaternion ),

			Ax::Parser::SKeyword( "BYTE",				kKeyword_UInt8 ),
			Ax::Parser::SKeyword( "WORD",				kKeyword_UInt16 ),
			Ax::Parser::SKeyword( "DWORD",				kKeyword_UInt32 ),
			Ax::Parser::SKeyword( "QWORD",				kKeyword_UInt64 ),
			Ax::Parser::SKeyword( "INTEGER",			kKeyword_Int32 ),
			Ax::Parser::SKeyword( "DOUBLE INTEGER",		kKeyword_Int64 ),
			Ax::Parser::SKeyword( "FLOAT",				kKeyword_Float32 ),
			Ax::Parser::SKeyword( "DOUBLE FLOAT",		kKeyword_Float64 ),

			Ax::Parser::SKeyword( "END",				kKeyword_RT_End ),
			Ax::Parser::SKeyword( "PRINT",				kKeyword_RT_Print ),
			Ax::Parser::SKeyword( "PRINTC",				kKeyword_RT_PrintC ),
			Ax::Parser::SKeyword( "DATA",				kKeyword_RT_Data ),
			Ax::Parser::SKeyword( "READ",				kKeyword_RT_Read ),
			Ax::Parser::SKeyword( "RESTORE",			kKeyword_RT_Restore ),

			Ax::Parser::SKeyword( "DIM",				kKeyword_RT_Dim ),
			Ax::Parser::SKeyword( "REDIM",				kKeyword_RT_Redim ),
			Ax::Parser::SKeyword( "UNDIM",				kKeyword_RT_Undim ),
			
			Ax::Parser::SKeyword( "NOT",				kKeyword_Op_Not ),
			Ax::Parser::SKeyword( "AND",				kKeyword_Op_And ),
			Ax::Parser::SKeyword( "OR",					kKeyword_Op_Or ),
			Ax::Parser::SKeyword( "XOR",				kKeyword_Op_Xor ),
			Ax::Parser::SKeyword( "MOD",				kKeyword_Op_Mod ),
			Ax::Parser::SKeyword( "BITWISE LSHIFT",		kKeyword_Op_BitwiseLShift ),
			Ax::Parser::SKeyword( "BITWISE RSHIFT",		kKeyword_Op_BitwiseRShift ),
			Ax::Parser::SKeyword( "BITWISE LROTATE",	kKeyword_Op_BitwiseLRotate ),
			Ax::Parser::SKeyword( "BITWISE RROTATE",	kKeyword_Op_BitwiseRRotate ),
			Ax::Parser::SKeyword( "BITWISE AND",		kKeyword_Op_BitwiseAnd ),
			Ax::Parser::SKeyword( "BITWISE OR",			kKeyword_Op_BitwiseOr ),
			Ax::Parser::SKeyword( "BITWISE XOR",		kKeyword_Op_BitwiseXor ),
			Ax::Parser::SKeyword( "BITWISE NOT",		kKeyword_Op_BitwiseNot )
		};

		SrcProc.AddKeywords( Keywords );
	}

	Ax::Parser::SKeyword *GetEndifKeywordPtr()
	{
		return GetDefaultSourceProcessor().FindKeyword( "endif" );
	}

}}
