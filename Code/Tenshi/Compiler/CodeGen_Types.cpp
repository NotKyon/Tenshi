#include "_PCH.hpp"
#include "CodeGen.hpp"
#include "Environment.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	void SCleanupScope::EmitCleanup( llvm::Value *pIgnoreVal )
	{
		auto &Builder = CG->Builder();

		for( uintptr i = Funcs.Num(); i > 0; --i ) {
			SCleanupFunction &F = Funcs[ i - 1 ];

			if( F.pValue == pIgnoreVal ) {
				continue;
			}

			Builder.CreateCall( F.pFunction, F.pValue );
		}
	}
	
	llvm::Type *MCodeGen::GetBuiltinType( EBuiltinType T ) const
	{
		llvm::LLVMContext &Ctx = m_Context;

		switch( T )
		{
		case EBuiltinType::Invalid:
		case EBuiltinType::UserDefined:
		case EBuiltinType::Any:
		case EBuiltinType::SIMDVector128:
		case EBuiltinType::SIMDVector256:
		case EBuiltinType::ArrayObject:
		case EBuiltinType::LinkedListObject:
		case EBuiltinType::BinaryTreeObject:
			break;

		case EBuiltinType::Void:
			return llvm::Type::getVoidTy( Ctx );

		case EBuiltinType::Boolean:
			return llvm::Type::getInt1Ty( Ctx );

		case EBuiltinType::Int8:
		case EBuiltinType::UInt8:
		case EBuiltinType::UNorm8:
		case EBuiltinType::SNorm8:
			return llvm::Type::getInt8Ty( Ctx );
		case EBuiltinType::Int16:
		case EBuiltinType::UInt16:
		case EBuiltinType::UNorm16:
		case EBuiltinType::SNorm16:
			return llvm::Type::getInt16Ty( Ctx );
		case EBuiltinType::Int32:
		case EBuiltinType::UInt32:
		case EBuiltinType::UNorm32:
		case EBuiltinType::SNorm32:
			return llvm::Type::getInt32Ty( Ctx );
		case EBuiltinType::Int64:
		case EBuiltinType::UInt64:
		case EBuiltinType::UNorm64:
		case EBuiltinType::SNorm64:
			return llvm::Type::getInt64Ty( Ctx );
		case EBuiltinType::Int128:
		case EBuiltinType::UInt128:
			return llvm::IntegerType::get( Ctx, 128 );

		case EBuiltinType::Float16:
			return llvm::Type::getHalfTy( Ctx );
		case EBuiltinType::Float32:
			return llvm::Type::getFloatTy( Ctx );
		case EBuiltinType::Float64:
			return llvm::Type::getDoubleTy( Ctx );

		case EBuiltinType::ConstUTF8Pointer:
		case EBuiltinType::StringObject:
			return llvm::Type::getInt8PtrTy( Ctx );
		case EBuiltinType::ConstUTF16Pointer:
			return llvm::Type::getInt16PtrTy( Ctx );

		case EBuiltinType::SIMDVector128_4Float32:
			return llvm::VectorType::get( llvm::Type::getFloatTy( Ctx ), 4 );
		case EBuiltinType::SIMDVector128_2Float64:
			return llvm::VectorType::get( llvm::Type::getDoubleTy( Ctx ), 2 );
		case EBuiltinType::SIMDVector128_4Int32:
			return llvm::VectorType::get( llvm::Type::getInt32Ty( Ctx ), 4 );
		case EBuiltinType::SIMDVector128_2Int64:
			return llvm::VectorType::get( llvm::Type::getInt64Ty( Ctx ), 2 );
		case EBuiltinType::SIMDVector256_8Float32:
			return llvm::VectorType::get( llvm::Type::getFloatTy( Ctx ), 8 );
		case EBuiltinType::SIMDVector256_4Float64:
			return llvm::VectorType::get( llvm::Type::getFloatTy( Ctx ), 4 );
		case EBuiltinType::SIMDVector256_8Int32:
			return llvm::VectorType::get( llvm::Type::getInt32Ty( Ctx ), 8 );
		case EBuiltinType::SIMDVector256_4Int64:
			return llvm::VectorType::get( llvm::Type::getInt64Ty( Ctx ), 4 );
		
		case EBuiltinType::Vector2f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 2 );
		case EBuiltinType::Vector3f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 3 );
		case EBuiltinType::Vector4f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 4 );

		case EBuiltinType::Matrix2f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 2*2 );
		case EBuiltinType::Matrix3f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 3*3 );
		case EBuiltinType::Matrix4f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 4*4 );
		case EBuiltinType::Matrix23f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 2*3 );
		case EBuiltinType::Matrix24f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 2*4 );
		case EBuiltinType::Matrix32f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 3*2 );
		case EBuiltinType::Matrix34f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 3*4 );
		case EBuiltinType::Matrix42f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 4*2 );
		case EBuiltinType::Matrix43f:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 4*3 );
		
		case EBuiltinType::Quaternionf:
			return llvm::ArrayType::get( llvm::Type::getFloatTy( Ctx ), 4 );
		}

		AX_ASSERT_MSG( false, "Invalid or unknown built-in type" );
		return nullptr;
	}

	llvm::Value *MCodeGen::GetDefaultConstant( EBuiltinType BTy )
	{
		AX_ASSERT( BTy != EBuiltinType::Invalid );
		AX_ASSERT( BTy != EBuiltinType::Any );
		AX_ASSERT( BTy != EBuiltinType::UserDefined );
		AX_ASSERT( BTy != EBuiltinType::Void );

		llvm::LLVMContext &C = m_Context;

		switch( BTy )
		{
		case EBuiltinType::Int8:
		case EBuiltinType::UInt8:
		case EBuiltinType::SNorm8:
		case EBuiltinType::UNorm8:
			return llvm::ConstantInt::get( llvm::Type::getInt8Ty( C ), llvm::APInt( 8, 0 ) );

		case EBuiltinType::Int16:
		case EBuiltinType::UInt16:
		case EBuiltinType::SNorm16:
		case EBuiltinType::UNorm16:
			return llvm::ConstantInt::get( llvm::Type::getInt16Ty( C ), llvm::APInt( 16, 0 ) );

		case EBuiltinType::Int32:
		case EBuiltinType::UInt32:
		case EBuiltinType::SNorm32:
		case EBuiltinType::UNorm32:
			return llvm::ConstantInt::get( llvm::Type::getInt32Ty( C ), llvm::APInt( 32, 0 ) );

		case EBuiltinType::Int64:
		case EBuiltinType::UInt64:
		case EBuiltinType::SNorm64:
		case EBuiltinType::UNorm64:
			return llvm::ConstantInt::get( llvm::Type::getInt64Ty( C ), llvm::APInt( 64, 0 ) );

		case EBuiltinType::Int128:
		case EBuiltinType::UInt128:
			return llvm::ConstantInt::get( llvm::Type::getIntNTy( C, 128 ), llvm::APInt( 128, 0 ) );

		case EBuiltinType::Boolean:
			return llvm::ConstantInt::get( llvm::Type::getInt1Ty( C ), llvm::APInt( 1, 0 ) );

		case EBuiltinType::Float16:
			return llvm::ConstantFP::get( llvm::Type::getHalfTy( C ), 0.0 );
		case EBuiltinType::Float32:
			return llvm::ConstantFP::get( llvm::Type::getFloatTy( C ), 0.0 );
		case EBuiltinType::Float64:
			return llvm::ConstantFP::get( llvm::Type::getDoubleTy( C ), 0.0 );

		case EBuiltinType::StringObject:
			return llvm::ConstantPointerNull::get( llvm::Type::getInt8PtrTy( C ) );
		}

		AX_ASSERT_MSG( false, "Unhandled EBuiltinType" );
		return nullptr;
	}

	void MCodeGen::EmitConstruct( llvm::Value *pStorePtr, const STypeRef &Type )
	{
		AX_ASSERT_NOT_NULL( pStorePtr );
		AX_ASSERT( Type.BuiltinType != EBuiltinType::Invalid );
		AX_ASSERT( Type.Translated.bDidTranslate == true );
		AX_ASSERT_NOT_NULL( Type.Translated.pType );

		AX_EXPECT_MSG( Type.BuiltinType != EBuiltinType::BinaryTreeObject, "Binary trees aren't supported yet" );
		AX_EXPECT_MSG( Type.BuiltinType != EBuiltinType::LinkedListObject, "Linked lists aren't supported yet" );

		if( Type.BuiltinType != EBuiltinType::UserDefined ) {
			llvm::Value *const pVal = GetDefaultConstant( Type.BuiltinType );
			if( !pVal ) {
				return;
			}

			m_IRBuilder.CreateStore( pVal, pStorePtr, Type.IsVolatile() );
			return;
		}

		AX_ASSERT_NOT_NULL( Type.pCustomType );

		static bool bDidDisplay = false; if( bDidDisplay ) { return; } else { bDidDisplay = true; }
		AX_DEBUG_LOG += "UDTs not constructed yet";
	}
	void MCodeGen::EmitDestruct( llvm::Value *pStorePtr, const STypeRef &Type )
	{
		AX_ASSERT_NOT_NULL( pStorePtr );
		AX_ASSERT( Type.BuiltinType != EBuiltinType::Invalid );
		AX_ASSERT( Type.Translated.bDidTranslate == true );
		AX_ASSERT_NOT_NULL( Type.Translated.pType );
		AX_ASSERT_NOT_NULL( m_IntFuncs.pStrReclaim );

		AX_EXPECT_MSG( Type.BuiltinType != EBuiltinType::BinaryTreeObject, "Binary trees aren't supported yet" );
		AX_EXPECT_MSG( Type.BuiltinType != EBuiltinType::LinkedListObject, "Linked lists aren't supported yet" );

		if( Type.BuiltinType == EBuiltinType::ArrayObject ) {
			AddCleanCall( m_IntFuncs.pArrayUndim, pStorePtr );
			return;
		}

		if( Type.BuiltinType != EBuiltinType::UserDefined ) {
			if( Type.BuiltinType == EBuiltinType::StringObject ) {
				AddCleanCall( m_IntFuncs.pStrReclaim, pStorePtr );
			}

			return;
		}

		AX_ASSERT_NOT_NULL( Type.pCustomType );

		static bool bDidDisplay = false; if( bDidDisplay ) { return; } else { bDidDisplay = true; }
		AX_DEBUG_LOG += "UDTs not destructed yet";
	}
	
	llvm::Value *MCodeGen::EmitDefaultInstance( const STypeRef &Type )
	{
		switch( Type.BuiltinType )
		{
		case EBuiltinType::Boolean:
			return llvm::ConstantInt::get( llvm::Type::getInt1Ty( m_Context ), llvm::APInt( 1, 0, false ) );
		case EBuiltinType::Int8:
		case EBuiltinType::UInt8:
		case EBuiltinType::SNorm8:
		case EBuiltinType::UNorm8:
			return llvm::ConstantInt::get( llvm::Type::getInt8Ty( m_Context ), llvm::APInt( 8, 0, false ) );
		case EBuiltinType::Int16:
		case EBuiltinType::UInt16:
		case EBuiltinType::SNorm16:
		case EBuiltinType::UNorm16:
			return llvm::ConstantInt::get( llvm::Type::getInt16Ty( m_Context ), llvm::APInt( 16, 0, false ) );
		case EBuiltinType::Int32:
		case EBuiltinType::UInt32:
		case EBuiltinType::SNorm32:
		case EBuiltinType::UNorm32:
			return llvm::ConstantInt::get( llvm::Type::getInt32Ty( m_Context ), llvm::APInt( 32, 0, false ) );
		case EBuiltinType::Int64:
		case EBuiltinType::UInt64:
		case EBuiltinType::SNorm64:
		case EBuiltinType::UNorm64:
			return llvm::ConstantInt::get( llvm::Type::getInt64Ty( m_Context ), llvm::APInt( 64, 0, false ) );
		case EBuiltinType::Int128:
		case EBuiltinType::UInt128:
			return llvm::ConstantInt::get( llvm::Type::getInt16Ty( m_Context ), llvm::APInt( 128, 0, false ) );
		case EBuiltinType::Float16:
			return llvm::ConstantFP::get( llvm::Type::getHalfTy( m_Context ), 0.0 );
		case EBuiltinType::Float32:
			return llvm::ConstantFP::get( llvm::Type::getFloatTy( m_Context ), 0.0 );
		case EBuiltinType::Float64:
			return llvm::ConstantFP::get( llvm::Type::getDoubleTy( m_Context ), 0.0 );
		case EBuiltinType::ConstUTF8Pointer:
			return m_IRBuilder.CreateGlobalStringPtr( "" );
		case EBuiltinType::ConstUTF16Pointer:
			return llvm::ConstantDataArray::get( m_Context, llvm::ArrayRef< uint16_t >(0) );
		case EBuiltinType::StringObject:
		case EBuiltinType::ArrayObject:
		case EBuiltinType::BinaryTreeObject:
			return llvm::ConstantPointerNull::get( llvm::Type::getInt8PtrTy( m_Context ) );
		}

		AX_ASSERT_MSG( false, "Unreachable (Unhandled built-in type for default instance code-gen)" );
		return nullptr;
	}

	unsigned MCodeGen::GetTypeId()
	{
		return m_uTypeId++;
	}
	llvm::Type *MCodeGen::GetRTTIType()
	{
		if( !m_pRTTITy ) {
			llvm::Type *const pInt8PtrTy = llvm::Type::getInt8PtrTy( m_Context );
			llvm::Type *const pVoidTy = llvm::Type::getVoidTy( m_Context );
			llvm::Type *const pBoolTy = llvm::Type::getInt1Ty( m_Context );
			llvm::Type *const pInt32Ty = llvm::Type::getInt32Ty( m_Context );

			llvm::Type *const pInitRetTy = pBoolTy;
			llvm::Type *const pInitArgTy[] = {
				pInt8PtrTy, pInt8PtrTy
			};
			llvm::Type *const pFiniRetTy = pVoidTy;
			llvm::Type *const pFiniArgTy[] = {
				pInt8PtrTy, pInt8PtrTy
			};
			llvm::Type *const pCopyRetTy = pVoidTy;
			llvm::Type *const pCopyArgTy[] = {
				pInt8PtrTy, pInt8PtrTy, pInt8PtrTy
			};
			llvm::Type *const pMoveRetTy = pVoidTy;
			llvm::Type *const pMoveArgTy[] = {
				pInt8PtrTy, pInt8PtrTy, pInt8PtrTy
			};

			// TenshiFnInstanceInit_t
			m_pObjInitFTy = llvm::FunctionType::get( pInitRetTy, pInitArgTy, false );
			// TenshiFnInstanceFini_t
			m_pObjFiniFTy = llvm::FunctionType::get( pFiniRetTy, pFiniArgTy, false );
			// TenshiFnInstanceCopy_t
			m_pObjCopyFTy = llvm::FunctionType::get( pCopyRetTy, pCopyArgTy, false );
			// TenshiFnInstanceMove_t
			m_pObjMoveFTy = llvm::FunctionType::get( pMoveRetTy, pMoveArgTy, false );

			// TenshiType_s (see TenshiRuntime.h)
			llvm::Type *const Elements[] = {
				// Flags
				pInt32Ty,
				// cBytes
				pInt32Ty,

				// pszName
				pInt8PtrTy,
				// pszPattern
				pInt8PtrTy,

				// pfnInit
				m_pObjInitFTy->getPointerTo(),
				// pfnFini
				m_pObjFiniFTy->getPointerTo(),
				// pfnCopy
				m_pObjCopyFTy->getPointerTo(),
				// pfnMove
				m_pObjMoveFTy->getPointerTo()
			};

			m_pRTTITy = llvm::StructType::get( m_Context, Elements, false );

			AX_EXPECT_NOT_NULL( m_pRTTITy );
		}

		return m_pRTTITy;
	}
	llvm::FunctionType *MCodeGen::GetObjInitFnTy()
	{
		( void )GetRTTIType();

		AX_ASSERT_NOT_NULL( m_pObjInitFTy );
		return m_pObjInitFTy;
	}
	llvm::FunctionType *MCodeGen::GetObjFiniFnTy()
	{
		( void )GetRTTIType();

		AX_ASSERT_NOT_NULL( m_pObjFiniFTy );
		return m_pObjFiniFTy;
	}
	llvm::FunctionType *MCodeGen::GetObjCopyFnTy()
	{
		( void )GetRTTIType();

		AX_ASSERT_NOT_NULL( m_pObjCopyFTy );
		return m_pObjCopyFTy;
	}
	llvm::FunctionType *MCodeGen::GetObjMoveFnTy()
	{
		( void )GetRTTIType();

		AX_ASSERT_NOT_NULL( m_pObjMoveFTy );
		return m_pObjMoveFTy;
	}

	void MCodeGen::RegisterUDT( STypeInfo &UDT )
	{
		AX_EXPECT_MSG( m_UserTypes.Append( &UDT ), "Out of memory" );
	}

	void MCodeGen::EmitReflectionData()
	{
#define TENSHI_RTTI_COUNT_SYM "__tenshi_rtti__count"
#define TENSHI_RTTI_ARRAY_SYM "__tenshi_rtti__array"

		llvm::Type *const pUInt32Ty = llvm::Type::getInt32Ty( m_Context );
		llvm::Type *const pUIntPtrTy = llvm::Type::getIntNTy( m_Context, g_Env->GetPointerBits() );

		llvm::Type *const pRTTITy = GetRTTIType();
		llvm::Type *const pRTTIPtrTy = pRTTITy->getPointerTo();

		llvm::Constant *const pZero = llvm::Constant::getNullValue( pUIntPtrTy );

		const unsigned cTypes = ( unsigned )m_UserTypes.Num();
		
		llvm::Value *pCountVal = nullptr;
		llvm::Value *pArrayVal = nullptr;
		
		pCountVal =
			new llvm::GlobalVariable
			(
				*m_pModule,
				pUInt32Ty,
				false,
				llvm::GlobalValue::LinkageTypes::AvailableExternallyLinkage,
				llvm::ConstantInt::get( pUInt32Ty, ( uint64_t )cTypes, false ),
				TENSHI_RTTI_COUNT_SYM
			);

		if( !cTypes ) {
			pArrayVal =
				new llvm::GlobalVariable
				(
					*m_pModule,
					pRTTIPtrTy,
					false,
					llvm::GlobalValue::LinkageTypes::AvailableExternallyLinkage,
					llvm::Constant::getNullValue( pRTTIPtrTy ),
					TENSHI_RTTI_ARRAY_SYM
				);

			return;
		}

		Ax::TArray< uint8_t > StringData;
		Ax::TArray< uintptr > StringOffs;

		Ax::String Pattern;

		for( STypeInfo *pUDT : m_UserTypes ) {
			AX_ASSERT_NOT_NULL( pUDT );
			STypeInfo &UDT = *pUDT;

			if( UDT.Members.IsEmpty() ) {
				continue;
			}

			AX_EXPECT_MSG( StringOffs.Append( StringData.Num() ), "Out of memory" );
			AX_EXPECT_MSG( StringData.Append( UDT.Name.Num(), ( const uint8_t * )UDT.Name.CString() ), "Out of memory" );
			AX_EXPECT_MSG( StringData.Append( '\0' ), "Out of memory" );

			Pattern = GetTypePattern( UDT.Members );
			AX_EXPECT_MSG( Pattern.Len() > 0, "Out of memory" );

			AX_EXPECT_MSG( StringOffs.Append( StringData.Num() ), "Out of memory" );
			AX_EXPECT_MSG( StringData.Append( Pattern.Num(), ( const uint8_t * )Pattern.CString() ), "Out of memory" );
			AX_EXPECT_MSG( StringData.Append( '\0' ), "Out of memory" );
		}

		llvm::Constant *const pStringBufferInit =
			llvm::ConstantDataArray::get
			(
				m_Context,
				llvm::ArrayRef<uint8_t>( StringData.Pointer(), StringData.Num() )
			);

		llvm::GlobalVariable *const pStringBuffer =
			new llvm::GlobalVariable
			(
				*m_pModule,
				pStringBufferInit->getType(),
				true,
				llvm::GlobalValue::LinkageTypes::InternalLinkage,
				pStringBufferInit
			);

		llvm::Constant *const pNullInitFn = llvm::Constant::getNullValue( GetObjInitFnTy()->getPointerTo() );
		llvm::Constant *const pNullFiniFn = llvm::Constant::getNullValue( GetObjFiniFnTy()->getPointerTo() );
		llvm::Constant *const pNullCopyFn = llvm::Constant::getNullValue( GetObjCopyFnTy()->getPointerTo() );
		llvm::Constant *const pNullMoveFn = llvm::Constant::getNullValue( GetObjFiniFnTy()->getPointerTo() );

		Ax::TArray< llvm::Constant * > RTTIData;
		AX_EXPECT_MSG( RTTIData.Reserve( cTypes ), "Out of memory" );

		uintptr uIndex = 0;
		for( STypeInfo *pUDT : m_UserTypes ) {
			AX_ASSERT_NOT_NULL( pUDT );
			STypeInfo &UDT = *pUDT;

			if( UDT.Members.IsEmpty() ) {
				continue;
			}

			llvm::Constant *const pOff1 = llvm::ConstantInt::get( pUIntPtrTy, ( uint64_t )StringOffs[ uIndex + 0 ], false );
			llvm::Constant *const pOff2 = llvm::ConstantInt::get( pUIntPtrTy, ( uint64_t )StringOffs[ uIndex + 1 ], false );
			uIndex += 2;

			llvm::Constant *const pIndexes1[] = { pZero, pOff1 };
			llvm::Constant *const pIndexes2[] = { pZero, pOff2 };

			unsigned uFlags = 0;
			if( UDT.bIsInitTrivial ) { uFlags |= 0x01; }
			if( UDT.bIsFiniTrivial ) { uFlags |= 0x02; }
			if( UDT.bIsCopyTrivial ) { uFlags |= 0x04; }
			if( UDT.bIsMoveTrivial ) { uFlags |= 0x08; }

			llvm::Constant *const pTyFlag = llvm::ConstantInt::get( pUInt32Ty, ( uint64_t )uFlags, false );
			llvm::Constant *const pTySize = llvm::ConstantInt::get( pUInt32Ty, ( uint64_t )UDT.cBytes, false );
			
			llvm::Constant *const pTyName = llvm::ConstantExpr::getInBoundsGetElementPtr( pStringBuffer, pIndexes1 );
			llvm::Constant *const pTyPtrn = llvm::ConstantExpr::getInBoundsGetElementPtr( pStringBuffer, pIndexes2 );

			llvm::Constant *const pTyInit = UDT.bIsInitTrivial ? pNullInitFn : UDT.pLLVMInitFn;
			llvm::Constant *const pTyFini = UDT.bIsFiniTrivial ? pNullFiniFn : UDT.pLLVMFiniFn;
			llvm::Constant *const pTyCopy = UDT.bIsCopyTrivial ? pNullCopyFn : UDT.pLLVMCopyFn;
			llvm::Constant *const pTyMove = UDT.bIsMoveTrivial ? pNullMoveFn : UDT.pLLVMMoveFn;

			llvm::Constant *const Members[] = {
				pTyFlag,
				pTySize,

				pTyName,
				pTyPtrn,

				pTyInit,
				pTyFini,
				pTyCopy,
				pTyMove
			};

			pRTTITy->dump();
			for( auto &x : Members ) {
				x->dump();
			}

			llvm::Constant *const pStructVal =
				llvm::ConstantStruct::get
				(
					llvm::cast< llvm::StructType >( pRTTITy ),
					llvm::ArrayRef< llvm::Constant * >( Members )
				);

			AX_EXPECT_MSG( RTTIData.Append( pStructVal ), "Out of memory" );
		}

		llvm::ArrayType *const pArrTy = llvm::ArrayType::get( pRTTITy, ( uint64_t )RTTIData.Num() );
		llvm::Constant *const pArrInit = llvm::ConstantArray::get( pArrTy, LLVMArr( RTTIData ) );

		pArrayVal =
			new llvm::GlobalVariable
			(
				*m_pModule,
				pArrTy,
				false,
				llvm::GlobalValue::LinkageTypes::AvailableExternallyLinkage,
				pArrInit,
				TENSHI_RTTI_ARRAY_SYM
			);

		( void )pArrayVal;
	}

}}
