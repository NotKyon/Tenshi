#pragma once

#include <Collections/List.hpp>

#include <Core/Manager.hpp>
#include <Core/String.hpp>
#include <Core/Types.hpp>

#include <System/FileSystem.hpp>

namespace Tenshi { namespace Compiler {

	enum ETestFlag
	{
		kTestF_Lexer				= 1<<0,
		kTestF_Preprocessor			= 1<<1,
		kTestF_Parser				= 1<<2,
		kTestF_CodeGenerator		= 1<<3,

		kTestF_All					=
										kTestF_Lexer |
										kTestF_Preprocessor |
										kTestF_Parser |
										kTestF_CodeGenerator
	};
	enum class ETestType
	{
		Unknown,
		Lexer,
		Preprocessor,
		Parser,
		CodeGenerator
	};

	class CTester
	{
	public:
		static CTester &GetInstance();
		~CTester();

		bool CollectTests( const Ax::String &Path );
		bool RunTests( Ax::uint32 Tests = kTestF_All );
		bool RunTest( const Ax::String &Filename, Ax::uint32 Tests = kTestF_All );

	private:
		Ax::System::CFileList		m_TestFiles;

		CTester();

		bool RunTest_Lexer( const Ax::String &Filename, Ax::TList< Ax::String > &TestLines, const Ax::String &TestFile );
		bool RunTest_Preprocessor( const Ax::String &Filename, Ax::TList< Ax::String > &TestLines, const Ax::String &TestFile );
		bool RunTest_Parser( const Ax::String &Filename, Ax::TList< Ax::String > &TestLines, const Ax::String &TestFile );
		bool RunTest_CodeGenerator( const Ax::String &Filename, Ax::TList< Ax::String > &TestLines, const Ax::String &TestFile );
	};
	static Ax::TManager< CTester >	Tester;

}}
