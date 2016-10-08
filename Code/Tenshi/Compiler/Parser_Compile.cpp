#include "_PCH.hpp"
#include "Parser.hpp"
#include "Symbol.hpp"
#include "TypeInformation.hpp"
#include "Environment.hpp"
#include "UDTParser.hpp"
#include "FunctionParser.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	//
	//	Run through the nodes, verifying semantics and collecting type
	//	information.
	//
	bool CParser::Semant()
	{
		// First semant all User-Defined-Types (UDTs) as they are allowed to be
		// anywhere in the program's scope (and accessed anywhere)
		for( CUserDefinedType *pType : m_Types ) {
			AX_ASSERT_NOT_NULL( pType );
			if( !pType->Semant() ) {
				return false;
			}
		}

		// Next semant all functions, but only their declarations. Their
		// definitions have to be semanted separately
		for( CFunctionDecl *pFunc : m_FuncDecls ) {
			AX_ASSERT_NOT_NULL( pFunc );
			if( !pFunc->PreSemant() ) {
				return false;
			}
		}

		// The main program can now be semanted (that includes code and global
		// declarations). Function declarations had to be semanted first as they
		// are allowed to be declared anywhere in program scope and accessed
		// from anywhere else.
		if( !m_Stmts.Semant() ) {
			return false;
		}

		// Finally, semant all functions separately. They need to be semanted
		// after the program's statements as the program may have globals which
		// the function will want to reference
		for( CFunctionDecl *pFunc : m_FuncDecls ) {
			AX_ASSERT_NOT_NULL( pFunc );
			if( !pFunc->Semant() ) {
				return false;
			}
		}

		return true;
	}

	//
	//	Run through the (already semanted) data and invoke the appropriate code
	//	generation routines
	//
	bool CParser::CodeGen()
	{
		// Do construction for UDTs now.
		//
		// NOTE: When constructors and destructors are added they need to be
		// -     done separately from this. Preferably in semantic analysis.
		for( CUserDefinedType *pType : m_Types ) {
			AX_ASSERT_NOT_NULL( pType );
			if( !pType->CodeGen() ) {
				return false;
			}
		}

		// Generate all function declarations
		for( CFunctionDecl *pFunc : m_FuncDecls ) {
			AX_ASSERT_NOT_NULL( pFunc );
			if( !pFunc->PreCodeGen() ) {
				return false;
			}
		}
		
		const bool bIsTopLevel = CG->EnterScope();
		AX_ASSERT( bIsTopLevel );

		// The main program should have its code generated before the
		// definitions of the other functions (so globals are referencable)
		if( !m_Stmts.CodeGen() ) {
			CG->LeaveScope();
			return false;
		}

		CG->CleanTopLevel();
		CG->LeaveScope();

		// Close up the main function now
		CG->CompleteMain();

		// Now do the same for all functions
		for( CFunctionDecl *pFunc : m_FuncDecls ) {
			AX_ASSERT_NOT_NULL( pFunc );
			if( !pFunc->CodeGen() ) {
				return false;
			}
		}

		// Save reflection data
		//CG->EmitReflectionData();

		// Finally, write out all of the module information we have
		CG->EmitModuleInfo();

		// Optimize the main function
		CG->OptimizeMain();

		// Done
		return true;
	}

}}
