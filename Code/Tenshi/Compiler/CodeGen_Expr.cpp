#include "_PCH.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	llvm::Value *MCodeGen::EmitCondition( CExpression &Expr )
	{
		SValue CondVal = Expr.CodeGen();
		if( !CondVal ) {
			Expr.Token().Error( "[CodeGen] Conditional expression did not evaluate" );
			return nullptr;
		}

		llvm::Type *const pCondTy = CondVal->getType();
		if( !pCondTy ) {
			Expr.Token().Error( "[CodeGen] Conditional expression has no type" );
			return nullptr;
		}

		llvm::Value *const pCondVal = CondVal.Load();

		if( pCondTy == llvm::Type::getInt1Ty( CG->Context() ) ) {
			return pCondVal;
		}

		if( pCondTy->isPointerTy() ) {
			return m_IRBuilder.CreateIsNotNull( pCondVal, "cmpnotnulltmp" );
		}

		if( pCondTy->isFloatingPointTy() ) {
			return m_IRBuilder.CreateFCmpONE( pCondVal, llvm::ConstantFP::get( m_Context, llvm::APFloat( 0.0 ) ) );
		}

		if( pCondTy->isIntegerTy() ) {
			return m_IRBuilder.CreateICmpNE( pCondVal, llvm::ConstantInt::get( m_Context, llvm::APInt( pCondTy->getScalarSizeInBits(), 0 ) ) );
		}

		Expr.Token().Error( "[CodeGen] Conditional expression has unhandled type" );
		return nullptr;
	}

	bool MCodeGen::EnterScope()
	{
		AX_EXPECT_MSG( m_CleanScopes.Append(), "Out of memory" );
		return m_CleanScopes.Num() == 1;
	}
	void MCodeGen::LeaveScope()
	{
		AX_ASSERT( !m_CleanScopes.IsEmpty() );
		m_CleanScopes.RemoveLast();
	}
	void MCodeGen::CleanScope( llvm::Value *pIgnoreVal )
	{
		AX_ASSERT( !m_CleanScopes.IsEmpty() );
		m_CleanScopes.Last().EmitCleanup( pIgnoreVal );
	}
	void MCodeGen::CleanTopLevel( llvm::Value *pIgnoreVal )
	{
		for( Ax::uintptr i = m_CleanScopes.Num(); i > 0; --i ) {
			SCleanupScope &Scope = m_CleanScopes[ i - 1 ];

			Scope.EmitCleanup( pIgnoreVal );
		}
	}
	void MCodeGen::AddCleanCall( llvm::Function *pFunc, llvm::Value *pArg )
	{
		AX_ASSERT( !m_CleanScopes.IsEmpty() );

		SCleanupScope &Scope = m_CleanScopes.Last();
		AX_EXPECT_MSG( Scope.Funcs.Append(), "Out of memory" );
	
		SCleanupFunction &CleanFunc = Scope.Funcs.Last();
		CleanFunc.pFunction = pFunc;
		CleanFunc.pValue = pArg;
	}
	void MCodeGen::RemoveCleanCall( llvm::Value *pArg )
	{
		if( m_CleanScopes.IsEmpty() ) {
			return;
		}

		SCleanupScope &Scope = m_CleanScopes.Last();

		uintptr i = Scope.Funcs.Num();
		while( i > 0 ) {
			--i;

			SCleanupFunction &CleanFunc = Scope.Funcs[ i ];

			if( CleanFunc.pValue == pArg ) {
				Scope.Funcs.Remove( i );
				break;
			}
		}
	}

}}
