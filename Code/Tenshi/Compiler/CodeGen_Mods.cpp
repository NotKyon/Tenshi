#include "_PCH.hpp"
#include "CodeGen.hpp"
#include "Module.hpp"
#include "Environment.hpp"
#include "Project.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	void MCodeGen::EmitModuleInfo()
	{
		llvm::Type *const pBoolTy = llvm::Type::getInt1Ty( m_Context );
		llvm::Type *const pVoidTy = llvm::Type::getVoidTy( m_Context );
		llvm::Type *const pUIntPtrTy = llvm::Type::getIntNTy( m_Context, ( unsigned int )g_Env->GetPointerSizeInBytes()*8 );
		llvm::Type *const pStrTy = llvm::Type::getInt8PtrTy( m_Context );

		llvm::Type *const Args[] = {
			pUIntPtrTy->getPointerTo()
		};

		llvm::FunctionType *const pModInitFnTy =
			llvm::FunctionType::get( pBoolTy, llvm::makeArrayRef( Args ), /*isVarArg=*/false );
		llvm::FunctionType *const pModFiniFnTy =
			llvm::FunctionType::get( pVoidTy, /*isVarArg=*/false );

		TArray<llvm::Constant *> ModNames, ModInits, ModFinis;

		/*

			!!! TODO !!!

			- Create a giant data buffer of all the module names.
			- Terminating NULs used between elements.
			- Fill an array with GEP'd pointers into the buffer.

		*/

		TArray<uint8> ModNameData;
		TArray<uintptr> DataOffsets;

		CProject &CurrProj = Projects->Current();

		for( SModule *pMod = CurrProj.m_Modules.Head(); pMod != nullptr; pMod = pMod->ProjectLink.Next() ) {
			if( pMod->Type == EModule::Internal ) {
				continue;
			}

			AX_EXPECT_MEMORY( DataOffsets.Append( ModNameData.Num() ) );

			AX_EXPECT_MEMORY( ModNameData.Append( pMod->Name.Num(), ( const uint8 * )pMod->Name.CString() ) );
			AX_EXPECT_MEMORY( ModNameData.Append( '\0' ) );
		}

		llvm::Constant *const pModNameBufferInit =
			llvm::ConstantDataArray::get
			(
				m_Context,
				llvm::ArrayRef<uint8_t>( ModNameData.Pointer(), ModNameData.Num() )
			);
		llvm::GlobalVariable *const pModNameBuffer =
			new llvm::GlobalVariable
			(
				*m_pModule,
				pModNameBufferInit->getType(),
				true,
				llvm::GlobalValue::LinkageTypes::InternalLinkage,
				pModNameBufferInit
			);

		uintptr uOffsetIndex = 0;
		for( SModule *pMod = CurrProj.m_Modules.Head(); pMod != nullptr; pMod = pMod->ProjectLink.Next() ) {
			if( pMod->Type == EModule::Internal ) {
				continue;
			}

			AX_EXPECT_MEMORY( ModNames.Append() );
			AX_EXPECT_MEMORY( ModInits.Append() );
			AX_EXPECT_MEMORY( ModFinis.Append() );

#if 0
			// FIXME: Unused. Seems like DataOffsets[] is used to reference the module name instead
			// Remove this?
			llvm::Constant *const pModNameConst =
				llvm::ConstantDataArray::getString( m_Context, LLVMStr( pMod->Name ), true );
#endif

			llvm::Constant *const pZero = llvm::Constant::getNullValue( pUIntPtrTy );
			llvm::Constant *const pOffs = llvm::ConstantInt::get( pUIntPtrTy, ( uint64_t )DataOffsets[ uOffsetIndex++ ], false );
			llvm::Constant *const pIndexes[] = { pZero, pOffs };

			llvm::Constant *const pModName =
					llvm::ConstantExpr::getInBoundsGetElementPtr
					(
						pModNameBufferInit->getType(),
						pModNameBuffer,
						llvm::makeArrayRef( pIndexes )
					);

			ModNames.Last() = pModName;

			ModInits.Last() =
				pMod->InitFunction.Name.IsEmpty()
				?
					( llvm::Constant * )llvm::ConstantPointerNull::get( pModInitFnTy->getPointerTo() )
				:
					llvm::Function::Create
					(
						pModInitFnTy,
						llvm::GlobalValue::ExternalLinkage,
						LLVMStr( pMod->InitFunction.Name ),
						m_pModule
					)
				;
			ModFinis.Last() =
				pMod->FiniFunction.Name.IsEmpty()
				?
					( llvm::Constant * )llvm::ConstantPointerNull::get( pModFiniFnTy->getPointerTo() )
				:
					llvm::Function::Create
					(
						pModFiniFnTy,
						llvm::GlobalValue::ExternalLinkage,
						LLVMStr( pMod->FiniFunction.Name ),
						m_pModule
					)
				;
		}

		const uintptr cModules = ModNames.Num();
		AX_ASSERT( ModNames.Num() == ModInits.Num() );
		AX_ASSERT( ModNames.Num() == ModFinis.Num() );

		// FIXME: The following is a HACK
		llvm::ArrayType *const pModNamesTy = llvm::ArrayType::get( pStrTy, cModules );
		llvm::ArrayType *const pModInitsTy = llvm::ArrayType::get( pModInitFnTy->getPointerTo(), cModules );
		llvm::ArrayType *const pModFinisTy = llvm::ArrayType::get( pModFiniFnTy->getPointerTo(), cModules );

		llvm::Constant *const pModNamesInit = llvm::ConstantArray::get( pModNamesTy, LLVMArr( ModNames ) );

		llvm::GlobalVariable *const pModNames =
			new llvm::GlobalVariable
			(
				*m_pModule,
				pModNamesTy,
				false,
				llvm::GlobalValue::LinkageTypes::ExternalLinkage,
				pModNamesInit,
				"tenshi__modNames__"
			);
		llvm::GlobalVariable *const pModInits =
			new llvm::GlobalVariable
			(
				*m_pModule,
				pModInitsTy,
				false,
				llvm::GlobalValue::LinkageTypes::ExternalLinkage,
				llvm::ConstantArray::get( pModInitsTy, LLVMArr( ModInits ) ),
				"tenshi__modInits__"
			);
		llvm::GlobalVariable *const pModFinis =
			new llvm::GlobalVariable
			(
				*m_pModule,
				pModFinisTy,
				false,
				llvm::GlobalValue::LinkageTypes::ExternalLinkage,
				llvm::ConstantArray::get( pModFinisTy, LLVMArr( ModFinis ) ),
				"tenshi__modFinis__"
			);

		llvm::GlobalVariable *const pNumMods =
			new llvm::GlobalVariable
			(
				*m_pModule,
				pUIntPtrTy,
				false,
				llvm::GlobalValue::LinkageTypes::ExternalLinkage,
				llvm::ConstantInt::get( pUIntPtrTy, ( uint64_t )cModules, false ),
				"tenshi__numMods__"
			);

		((void)pModNames);
		((void)pModInits);
		((void)pModFinis);
		((void)pNumMods);
	}

}}
