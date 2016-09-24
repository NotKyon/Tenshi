#include "_PCH.hpp"
#include "CodeGen.hpp"
#include "Environment.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	static llvm::Type *GetLLVMType( char chPatternType )
	{
		switch( chPatternType )
		{
		case '0':	return llvm::Type::getVoidTy( CG->Context() );
		case 'B':	return llvm::Type::getInt1Ty( CG->Context() );
		case 'Y':	return llvm::Type::getInt8Ty( CG->Context() );
		case 'W':	return llvm::Type::getInt16Ty( CG->Context() );
		case 'D':	return llvm::Type::getInt32Ty( CG->Context() );
		case 'Q':	return llvm::Type::getInt64Ty( CG->Context() );
		case 'Z':	return llvm::Type::getIntNTy( CG->Context(), 128 );
		case 'H':	return llvm::Type::getHalfTy( CG->Context() );
		case 'F':	return llvm::Type::getFloatTy( CG->Context() );
		case 'O':	return llvm::Type::getDoubleTy( CG->Context() );
		case 'S':	return llvm::Type::getInt8PtrTy( CG->Context() );
		case 'T':	return llvm::Type::getInt16PtrTy( CG->Context() );
		case 'U':	return llvm::Type::getIntNTy( CG->Context(), g_Env->GetPointerBits() );
		case 'P':	return llvm::Type::getInt8PtrTy( CG->Context() );
		}

		AX_ASSERT_MSG( false, "Unexpected type" );
		return nullptr;
	}

	static llvm::Function *MakeIntFunc( const char *pszName, char chReturn, const char *pszTypePattern )
	{
		static const uintptr kMaxArgs = 8;
		llvm::Type *Args[ kMaxArgs ];
		uintptr cArgs = 0;

		AX_ASSERT_NOT_NULL( pszName );
		AX_ASSERT_NOT_NULL( pszTypePattern );

		while( *pszTypePattern ) {
			AX_ASSERT( cArgs < kMaxArgs );

			Args[ cArgs++ ] = GetLLVMType( *pszTypePattern++ );
		}

		llvm::FunctionType *const pFuncType =
			llvm::FunctionType::get
			(
				GetLLVMType( chReturn ),
				llvm::ArrayRef< llvm::Type * >( Args, cArgs ),
				false
			);

		AX_ASSERT_NOT_NULL( pFuncType );

		llvm::Function *const pFunc =
			llvm::Function::Create
			(
				pFuncType,
				llvm::Function::ExternalLinkage,
				pszName,
				&CG->Module()
			);

		AX_ASSERT_NOT_NULL( pFunc );

		return pFunc;
	}

	void MCodeGen::Init()
	{
		AX_ASSERT( !IsInitialized() );

		m_uStringId = 0;
		m_uTypeId = 0;

		m_pRTTITy = nullptr;
		m_pObjInitFTy = nullptr;
		m_pObjFiniFTy = nullptr;
		m_pObjCopyFTy = nullptr;
		m_pObjMoveFTy = nullptr;

		if( !m_pPassReg ) {
			LLVMInitializeX86Target();
			LLVMInitializeX86TargetInfo();
			LLVMInitializeX86TargetMC();
			LLVMInitializeX86AsmPrinter();
			LLVMInitializeX86AsmParser();

			m_pPassReg = llvm::PassRegistry::getPassRegistry();
			AX_EXPECT_NOT_NULL( m_pPassReg );

			llvm::initializeCore( *m_pPassReg );
			llvm::initializeCodeGen( *m_pPassReg );
			llvm::initializeLoopStrengthReducePass( *m_pPassReg );
			llvm::initializeLowerIntrinsicsPass( *m_pPassReg );
			llvm::initializeUnreachableBlockElimPass( *m_pPassReg );
		}

		const std::string TripleName = llvm::Triple::normalize( "x86_64-pc-win32" );
		AX_DEBUG_LOG += TripleName.c_str();

		llvm::Triple Triple( TripleName );

		{
			std::string ErrorStr; // Dammit LLVM, really?
			m_pTarget = llvm::TargetRegistry::lookupTarget( "x86-64", Triple, ErrorStr );

			if( !m_pTarget ) {
				g_ErrorLog += ErrorStr.c_str();
				exit( EXIT_FAILURE );
			}
		}

		Triple = Triple.get64BitArchVariant();

		llvm::TargetOptions Opts;
		Opts.FloatABIType = llvm::FloatABI::Default;
		Opts.ThreadModel = llvm::ThreadModel::POSIX;
		Opts.MCOptions.AsmVerbose = true;
		Opts.MCOptions.ShowMCEncoding = true;

		m_pTargetMachine = m_pTarget->createTargetMachine( TripleName, "x86-64", "", Opts );
		AX_EXPECT_MEMORY( m_pTargetMachine );

		m_pModule = new llvm::Module( "", m_Context );
		AX_EXPECT_MEMORY( m_pModule );

		m_pModule->setTargetTriple( TripleName );
		m_pModule->setDataLayout( m_pTargetMachine->createDataLayout() );

		m_pObjFile = nullptr;
		m_pAsmFile = nullptr;

		m_pPM = new llvm::legacy::PassManager();
		AX_EXPECT_MEMORY( m_pPM );

		m_pFPM = new llvm::legacy::FunctionPassManager( m_pModule );
		AX_EXPECT_MEMORY( m_pFPM );

		/*
		llvm::DataLayoutPass *pDLPass = new llvm::DataLayoutPass();
		AX_EXPECT_MEMORY( pDLPass );

		m_pPM->add( pDLPass );
		*/

		m_pFPM->add( llvm::createVerifierPass() );
		//m_pFPM->add( llvm::createBasicAliasAnalysisPass() );
		m_pFPM->add( llvm::createInstructionCombiningPass() );
		m_pFPM->add( llvm::createReassociatePass() );
		m_pFPM->add( llvm::createGVNPass() );
		m_pFPM->add( llvm::createCFGSimplificationPass() );
		m_pFPM->doInitialization(); //don't worry about the return value here -- it just says whether it changed...

		m_IntFuncs.pAutoprint			= MakeIntFunc( "teAutoprint"        , '0', "S"  );
		m_IntFuncs.pSafeSync			= MakeIntFunc( "teSafeSync"         , 'B', ""   );
		m_IntFuncs.pStrDup				= MakeIntFunc( "teStrDup"           , 'S', "S"  );
		m_IntFuncs.pStrConcat			= MakeIntFunc( "teStrConcat"        , 'S', "SS" );
		m_IntFuncs.pStrFindRm			= MakeIntFunc( "teStrFindRm"        , 'S', "SS" );
		m_IntFuncs.pStrRepeat			= MakeIntFunc( "teStrRepeat"        , 'S', "SU" );
		m_IntFuncs.pStrCatDir			= MakeIntFunc( "teStrCatDir"        , 'S', "SS" );
		m_IntFuncs.pStrReclaim			= MakeIntFunc( "teStrReclaim"       , '0', "S"  );
		m_IntFuncs.pCastInt8ToStr		= MakeIntFunc( "teCastInt8ToStr"    , 'S', "Y"  );
		m_IntFuncs.pCastInt16ToStr		= MakeIntFunc( "teCastInt16ToStr"   , 'S', "W"  );
		m_IntFuncs.pCastInt32ToStr		= MakeIntFunc( "teCastInt32ToStr"   , 'S', "D"  );
		m_IntFuncs.pCastInt64ToStr		= MakeIntFunc( "teCastInt64ToStr"   , 'S', "Q"  );
		m_IntFuncs.pCastInt128ToStr		= MakeIntFunc( "teCastInt128ToStr"  , 'S', "Z"  );
		m_IntFuncs.pCastUInt8ToStr		= MakeIntFunc( "teCastUInt8ToStr"   , 'S', "Y"  );
		m_IntFuncs.pCastUInt16ToStr		= MakeIntFunc( "teCastUInt16ToStr"  , 'S', "W"  );
		m_IntFuncs.pCastUInt32ToStr		= MakeIntFunc( "teCastUInt32ToStr"  , 'S', "D"  );
		m_IntFuncs.pCastUInt64ToStr		= MakeIntFunc( "teCastUInt64ToStr"  , 'S', "Q"  );
		m_IntFuncs.pCastUInt128ToStr	= MakeIntFunc( "teCastUInt128ToStr" , 'S', "Z"  );
		m_IntFuncs.pCastFloat16ToStr	= MakeIntFunc( "teCastFloat16ToStr" , 'S', "H"  );
		m_IntFuncs.pCastFloat32ToStr	= MakeIntFunc( "teCastFloat32ToStr" , 'S', "F"  );
		m_IntFuncs.pCastFloat64ToStr	= MakeIntFunc( "teCastFloat64ToStr" , 'S', "O"  );
		//m_IntFuncs.pCastUTF8PtrToStr	= MakeIntFunc( "teCastUTF8PtrToStr" , 'S', "S"  );
		m_IntFuncs.pCastUTF16PtrToStr	= MakeIntFunc( "teCastUTF16PtrToStr", 'S', "T"  );

		m_IntFuncs.pArrayDim			= MakeIntFunc( "teArrayDim"         , 'P', "PUP" );		// pDimensions, cDimensions, pItemType
		m_IntFuncs.pArrayRedim			= MakeIntFunc( "teArrayRedim"       , 'P', "PPUP" );	// pOldArrayData, pDimensions, cDimensions, pItemType
		m_IntFuncs.pArrayUndim			= MakeIntFunc( "teArrayUndim"       , 'P', "P" );		// pArrayData
		m_IntFuncs.pArrayGetDimRes		= MakeIntFunc( "teArrayDimensionLen", 'U', "PU" );		// pArrayData, uDim
		m_IntFuncs.pArrayGetCurIdx		= MakeIntFunc( "teArrayCurrentIndex", 'U', "P" );		// pArrayData

		m_pEntryFunc =
			llvm::Function::Create
			(
				llvm::FunctionType::get
				(
					llvm::Type::getVoidTy( m_Context ),
					false
				),
				llvm::GlobalValue::LinkageTypes::ExternalLinkage,
				"TenshiMain",
				m_pModule
			);
		AX_EXPECT_MEMORY( m_pEntryFunc );

		m_pCurrentFunc = m_pEntryFunc;

		m_pCurrentBlock = llvm::BasicBlock::Create( m_Context, "entry", m_pEntryFunc );
		AX_EXPECT_MEMORY( m_pCurrentBlock );

		m_IRBuilder.SetInsertPoint( m_pCurrentBlock );
	}
	bool MCodeGen::IsInitialized() const
	{
		return m_pModule != nullptr;
	}
	void MCodeGen::Fini()
	{
		AX_ASSERT( IsInitialized() );

		m_pCurrentBlock = nullptr;
		m_pCurrentFunc = nullptr;

		delete m_pFPM;
		m_pFPM = nullptr;

		delete m_pPM;
		m_pPM = nullptr;

		delete m_pEntryFunc;
		m_pEntryFunc = nullptr;

		delete m_pModule;
		m_pModule = nullptr;
	}

	void MCodeGen::CompleteMain()
	{
		AX_ASSERT_NOT_NULL( m_pEntryFunc );
		AX_ASSERT( m_pEntryFunc->getBasicBlockList().empty() == false );

#if 0
		llvm::BasicBlock &LastBlock = m_pEntryFunc->getBasicBlockList().back();
#else
		AX_ASSERT_NOT_NULL( m_pCurrentBlock );
		llvm::BasicBlock &LastBlock = *m_pCurrentBlock;
#endif
		const bool bNeedTerminator = !LastBlock.getTerminator();

		if( bNeedTerminator ) {
			llvm::ReturnInst::Create( m_Context, &LastBlock );
		}
	}
	void MCodeGen::OptimizeMain()
	{
		AX_ASSERT_NOT_NULL( m_pFPM );
		AX_ASSERT_NOT_NULL( m_pEntryFunc );

		if( m_bOptimize ) {
			m_pFPM->run( *m_pEntryFunc );
		}
	}

}}
