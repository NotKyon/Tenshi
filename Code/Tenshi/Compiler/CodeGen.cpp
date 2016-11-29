#include "_PCH.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	MCodeGen &MCodeGen::GetInstance()
	{
		static MCodeGen instance;
		return instance;
	}

	MCodeGen::MCodeGen()
	: m_Context()
	, m_pPassReg( nullptr )
	, m_pModule( nullptr )
	, m_IRBuilder( m_Context )
	, m_pEntryFunc( nullptr )
	, m_pCurrentFunc( nullptr )
	, m_pCurrentBlock( nullptr )
	, m_pPM( nullptr )
	, m_pFPM( nullptr )
	, m_pTarget( nullptr )
	, m_pTargetMachine( nullptr )
	, m_pObjFile( nullptr )
	, m_pAsmFile( nullptr )
	, m_IntFuncs()
	, m_bOptimize( false )
	, m_bLabelDebugOut( false )
	, m_uStringId( 0 )
	, m_uTypeId( 0 )
	, m_pRTTITy( nullptr )
	, m_pObjInitFTy( nullptr )
	, m_pObjFiniFTy( nullptr )
	, m_pObjCopyFTy( nullptr )
	, m_pObjMoveFTy( nullptr )
	, m_LoopPoints()
	{
	}
	MCodeGen::~MCodeGen()
	{
		// FIXME: Crashes at program exit for tests
#if 0
		if( IsInitialized() ) {
			Fini();
		}
#endif
	}

	void MCodeGen::SetOptimize( bool bEnabled )
	{
		m_bOptimize = bEnabled;
	}
	void MCodeGen::SetLabelDebugLogging( bool bEnabled )
	{
		m_bLabelDebugOut = bEnabled;
	}
	bool MCodeGen::CanOptimize() const
	{
		return m_bOptimize;
	}
	bool MCodeGen::AreLabelsDebugLogged() const
	{
		return m_bLabelDebugOut;
	}

	// TODO: Is this used anywhere?
	llvm::StoreInst *MCodeGen::EmitStore( const SSymbol &DstVar, CExpression &Expr )
	{

		AX_ASSERT_NOT_NULL( DstVar.pVar );
		AX_ASSERT_NOT_NULL( DstVar.Translated.pValue );

		SValue ExprVal = Expr.CodeGen();
		if( !ExprVal ) {
			return nullptr;
		}

		const STypeRef *const pExprTypeRef = Expr.GetType();
		if( !const_cast< STypeRef * >( pExprTypeRef )->CodeGen() ) {
			return nullptr;
		}
		AX_ASSERT( pExprTypeRef->Translated.bDidTranslate );
		AX_ASSERT_NOT_NULL( pExprTypeRef->Translated.pType );

		llvm::Type *const pExprType = pExprTypeRef->Translated.pType;
		const bool bIsVolatile = !DstVar.pVar->Type.bCanReorderMemAccesses;

		//
		//	TODO: Generate an implicit cast if possible
		//
		( void )pExprType;

		llvm::Value *const pExprVal = ExprVal.Load();

		llvm::StoreInst *const pStore = m_IRBuilder.CreateStore( pExprVal, DstVar.Translated.pValue, bIsVolatile );

		if( pExprTypeRef->BuiltinType == EBuiltinType::StringObject ) {
			RemoveCleanCall( pExprVal );
		}

		return pStore;
	}
	llvm::CallInst *MCodeGen::EmitAutoprintCall( llvm::Value *pParm )
	{
		// TODO!
		return nullptr;
	}

	unsigned MCodeGen::GetStringId()
	{
		return m_uStringId++;
	}

}}
