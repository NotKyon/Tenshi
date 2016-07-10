#include "_PCH.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;
	
	llvm::Value *MCodeGen::EmitCast( ECast CastOp, EBuiltinType DstType, llvm::Value *pSrcVal )
	{
		llvm::Type *const pDstType = DstType == EBuiltinType::UserDefined ? pSrcVal->getType() : GetBuiltinType( DstType );
		AX_ASSERT_NOT_NULL( pDstType );

		llvm::CallInst *pInst = nullptr;

		switch( CastOp )
		{
		case ECast::Invalid:
			AX_ASSERT_MSG( false, "Invalid cast" );
			return nullptr;

		case ECast::None:
			return pSrcVal;

		case ECast::SignExtend:
			return m_IRBuilder.CreateSExt( pSrcVal, pDstType, "signexttmp" );
		case ECast::ZeroExtend:
			return m_IRBuilder.CreateZExt( pSrcVal, pDstType, "zeroexttmp" );
		case ECast::RealExtend:
			return m_IRBuilder.CreateFPExt( pSrcVal, pDstType, "realexttmp" );
		case ECast::SignTrunc:
		case ECast::ZeroTrunc:
			return m_IRBuilder.CreateTrunc( pSrcVal, pDstType, "trunctmp" );
		case ECast::RealTrunc:
			return m_IRBuilder.CreateFPTrunc( pSrcVal, pDstType, "realtrunctmp" );

		case ECast::SignedIntToFloat:
			return m_IRBuilder.CreateSIToFP( pSrcVal, pDstType, "sinttofloattmp" );
		case ECast::UnsignedIntToFloat:
			return m_IRBuilder.CreateUIToFP( pSrcVal, pDstType, "uinttofloattmp" );
		case ECast::FloatToSignedInt:
			return m_IRBuilder.CreateFPToSI( pSrcVal, pDstType, "floattosinttmp" );
		case ECast::FloatToUnsignedInt:
			return m_IRBuilder.CreateFPToUI( pSrcVal, pDstType, "floattouinttmp" );

		case ECast::IntToBool:
			return m_IRBuilder.CreateICmpNE( pSrcVal, llvm::Constant::getNullValue( pSrcVal->getType() ) );
		case ECast::FloatToBool:
			return m_IRBuilder.CreateFCmpONE( pSrcVal, llvm::Constant::getNullValue( pSrcVal->getType() ) );
		case ECast::PtrToBool:
			return m_IRBuilder.CreateIsNotNull( pSrcVal );
			
		case ECast::Int8ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastInt8ToStr );

			if( pSrcVal->getType() == llvm::Type::getInt1Ty( m_Context ) ) {
				pSrcVal = m_IRBuilder.CreateZExt( pSrcVal, llvm::Type::getInt8Ty( m_Context ) );
			}

			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastInt8ToStr, pSrcVal, "sbytetostrtmp" );
			break;
		case ECast::Int16ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastInt16ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastInt16ToStr, pSrcVal, "swordtostrtmp" );
			break;
		case ECast::Int32ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastInt32ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastInt32ToStr, pSrcVal, "sdwordtostrtmp" );
			break;
		case ECast::Int64ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastInt64ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastInt64ToStr, pSrcVal, "sqwordtostrtmp" );
			break;
		case ECast::Int128ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastInt128ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastInt128ToStr, pSrcVal, "sowordtostrmp" );
			break;
		case ECast::UInt8ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastUInt8ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastUInt8ToStr, pSrcVal, "ubytetostrtmp" );
			break;
		case ECast::UInt16ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastUInt16ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastUInt16ToStr, pSrcVal, "uwordtostrtmp" );
			break;
		case ECast::UInt32ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastUInt32ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastUInt32ToStr, pSrcVal, "udwordtostrtmp" );
			break;
		case ECast::UInt64ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastUInt64ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastUInt64ToStr, pSrcVal, "uqwordtostrtmp" );
			break;
		case ECast::UInt128ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastUInt128ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastUInt128ToStr, pSrcVal, "uowordtostrmp" );
			break;

		case ECast::Float16ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastFloat16ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastFloat16ToStr, pSrcVal, "halftostrtmp" );
			break;
		case ECast::Float32ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastFloat32ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastFloat32ToStr, pSrcVal, "floattostrtmp" );
			break;
		case ECast::Float64ToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastFloat64ToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastFloat64ToStr, pSrcVal, "doubletostrtmp" );
			break;

		case ECast::UTF8PtrToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pStrDup );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pStrDup, pSrcVal, "utfastrtmp" );
			break;
		case ECast::UTF16PtrToStr:
			AX_ASSERT_NOT_NULL( m_IntFuncs.pCastUTF16PtrToStr );
			pInst = m_IRBuilder.CreateCall( m_IntFuncs.pCastUTF16PtrToStr, pSrcVal, "utfwstrtmp" );
			break;
		}

		AX_ASSERT_NOT_NULL( pInst );

		AddCleanCall( m_IntFuncs.pStrReclaim, pInst );
		return pInst;
	}

}}
