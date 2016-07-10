#include "_PCH.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	void MCodeGen::Dump() const
	{
		AX_ASSERT_NOT_NULL( m_pModule );
		m_pModule->dump();
	}

	bool MCodeGen::AddObjOut( llvm::StringRef ObjFilename )
	{
		AX_ASSERT_IS_NULL( m_pObjFile );
		AX_ASSERT_NOT_NULL( m_pPM );
		AX_ASSERT_NOT_NULL( m_pTargetMachine );

		const char *const file_p = ObjFilename.data();
		const std::size_t file_n = ObjFilename.size();

		std::error_code EC;
		m_pObjFile = new llvm::tool_output_file( ObjFilename, EC, llvm::sys::fs::F_None );
		AX_EXPECT_MEMORY( m_pObjFile );
		if( EC ) {
			Ax::Errorf( Ax::String::Formatted( "%.*s", file_n, file_p ), "%s", EC.message().c_str() );
			return false;
		}

		return true;
	}
	bool MCodeGen::AddAsmOut( llvm::StringRef AsmFilename )
	{
		AX_ASSERT_IS_NULL( m_pAsmFile );
		AX_ASSERT_NOT_NULL( m_pPM );
		AX_ASSERT_NOT_NULL( m_pTargetMachine );

		const char *const file_p = AsmFilename.data();
		const std::size_t file_n = AsmFilename.size();

		std::error_code EC;
		m_pAsmFile = new llvm::tool_output_file( AsmFilename, EC, llvm::sys::fs::F_Text );
		AX_EXPECT_MEMORY( m_pAsmFile );
		if( EC ) {
			Ax::Errorf( Ax::String::Formatted( "%.*s", file_n, file_p ), "%s", EC.message().c_str() );
			return false;
		}

		return true;
	}

	bool MCodeGen::WriteOutputs()
	{
		AX_ASSERT_NOT_NULL( m_pModule );
		AX_ASSERT_NOT_NULL( m_pPM );

		m_pPM->run( *m_pModule );

		if( m_pAsmFile != nullptr ) {
			llvm::PassManager AsmPM;
			llvm::formatted_raw_ostream AsmFOS( m_pAsmFile->os() );

			//AsmPM.add( new llvm::DataLayoutPass() );

			if( m_pTargetMachine->addPassesToEmitFile( AsmPM, AsmFOS, llvm::TargetMachine::CGFT_AssemblyFile ) ) {
				Ax::BasicErrorf( "Target does not support assembly output" );
				return false;
			}

			AsmPM.run( *m_pModule );
			m_pAsmFile->keep();
		}

		delete m_pAsmFile;
		m_pAsmFile = nullptr;

		if( m_pObjFile != nullptr ) {
			llvm::PassManager ObjPM;
			llvm::formatted_raw_ostream ObjFOS( m_pObjFile->os() );

			//ObjPM.add( new llvm::DataLayoutPass() );

			if( m_pTargetMachine->addPassesToEmitFile( ObjPM, ObjFOS, llvm::TargetMachine::CGFT_ObjectFile ) ) {
				Ax::BasicErrorf( "Target does not support object output" );
				return false;
			}

			ObjPM.run( *m_pModule );
			m_pObjFile->keep();
		}

		delete m_pObjFile;
		m_pObjFile = nullptr;

		return true;
	}
	bool MCodeGen::WriteIR( llvm::StringRef IRFilename )
	{
		std::error_code EC;
		llvm::raw_fd_ostream OS( IRFilename, EC, llvm::sys::fs::F_Text );
		if( EC ) {
			return false;
		}

		OS << *m_pModule;
		return true;
	}
	bool MCodeGen::WriteBC( llvm::StringRef BCFilename )
	{
		std::error_code EC;
		llvm::raw_fd_ostream OS( BCFilename, EC, llvm::sys::fs::F_None );
		if( EC ) {
			return false;
		}

		llvm::WriteBitcodeToFile( m_pModule, OS );
		return true;
	}

}}
