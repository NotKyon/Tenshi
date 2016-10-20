#pragma once

#include <Collections/Array.hpp>
#include <Core/Manager.hpp>
#include <Parser/Token.hpp>

#include "BuiltinType.hpp"
#include "TypeInformation.hpp"
#include "Node.hpp"
#include "Symbol.hpp"

namespace Tenshi { namespace Compiler {

	typedef Ax::TArray< SMemberInfo >		MemberArray;

	inline llvm::StringRef LLVMStr( const char *p, size_t c )
	{
		const char *const s = p != nullptr ? p : "";
		const size_t n = c != ~size_t( 0 ) ? c : strlen( s );

		return llvm::StringRef( s, n );
	}
	inline llvm::StringRef LLVMStr( const Ax::String &x )
	{
		return llvm::StringRef( x.CString(), x.Len() );
	}
	inline llvm::StringRef LLVMStr( const Ax::Parser::SToken &Tok )
	{
		return llvm::StringRef( Tok.GetPointer(), Tok.cLength );
	}
	inline llvm::StringRef LLVMStr( const Ax::Parser::SToken *pTok )
	{
		return pTok != nullptr ? LLVMStr( *pTok ) : llvm::StringRef();
	}

	template< typename tElement >
	inline llvm::ArrayRef< tElement > LLVMArr( const Ax::TArray< tElement > &InArr )
	{
		return llvm::ArrayRef< tElement >( InArr.Pointer(), InArr.Num() );
	}

	struct SInternalFunctions
	{
		llvm::Function *			pAutoprint;
		llvm::Function *			pSafeSync;
		llvm::Function *			pStrDup;
		llvm::Function *			pStrConcat;
		llvm::Function *			pStrFindRm;
		llvm::Function *			pStrRepeat;
		llvm::Function *			pStrCatDir;
		llvm::Function *			pStrReclaim;
		llvm::Function *			pStrEq;
		llvm::Function *			pStrCmp;
		llvm::Function *			pCastInt8ToStr;
		llvm::Function *			pCastInt16ToStr;
		llvm::Function *			pCastInt32ToStr;
		llvm::Function *			pCastInt64ToStr;
		llvm::Function *			pCastInt128ToStr;
		llvm::Function *			pCastUInt8ToStr;
		llvm::Function *			pCastUInt16ToStr;
		llvm::Function *			pCastUInt32ToStr;
		llvm::Function *			pCastUInt64ToStr;
		llvm::Function *			pCastUInt128ToStr;
		llvm::Function *			pCastFloat16ToStr;
		llvm::Function *			pCastFloat32ToStr;
		llvm::Function *			pCastFloat64ToStr;
		//llvm::Function *			pCastUTF8PtrToStr;
		llvm::Function *			pCastUTF16PtrToStr;

		llvm::Function *			pArrayDim;
		llvm::Function *			pArrayRedim;
		llvm::Function *			pArrayUndim;

		llvm::Function *			pArrayGetDimRes;
		llvm::Function *			pArrayGetCurIdx;
	};

	struct SCleanupFunction
	{
		llvm::Value *				pValue;
		llvm::Function *			pFunction;
	};
	struct SCleanupScope
	{
		// The clean-up functions
		Ax::TArray< SCleanupFunction >	Funcs;

		// Clean-up the scope
		void EmitCleanup( llvm::Value *pIgnoreVal = nullptr );
	};

	class MCodeGen
	{
	public:
		static MCodeGen &GetInstance();

		void Init();
		bool IsInitialized() const;
		void Fini();

		void Dump() const;

		bool AddObjOut( llvm::StringRef ObjFilename );
		bool AddAsmOut( llvm::StringRef AsmFilename );

		bool WriteOutputs();
		bool WriteIR( llvm::StringRef IRFilename );
		bool WriteBC( llvm::StringRef BCFilename );

		inline llvm::LLVMContext &Context()
		{
			return m_Context;
		}
		inline llvm::Module &Module()
		{
			AX_ASSERT_NOT_NULL( m_pModule );
			return *m_pModule;
		}
		inline llvm::IRBuilder<> &Builder()
		{
			return m_IRBuilder;
		}
		inline llvm::legacy::FunctionPassManager &FPM()
		{
			AX_ASSERT_NOT_NULL( m_pFPM );
			return *m_pFPM;
		}

		void CompleteMain();
		void OptimizeMain();

		void EmitReflectionData();

		void SetOptimize( bool bEnabled );
		void SetLabelDebugLogging( bool bEnabled );
		bool CanOptimize() const;
		bool AreLabelsDebugLogged() const;

		bool HasCurrentBlock() const;
		llvm::BasicBlock &CurrentBlock();
		void SetCurrentBlock( llvm::BasicBlock &Block );

		bool HasCurrentFunction() const;
		llvm::Function &CurrentFunction();

		llvm::Type *GetBuiltinType( EBuiltinType T ) const;

		llvm::BasicBlock &DeclareLabel( const char *pLabelName = nullptr, size_t cLabelName = 0 );
		inline llvm::BasicBlock &DeclareLabel( const Ax::String &LabelName )
		{
			return DeclareLabel( LabelName.CString(), LabelName.Len() );
		}
		inline llvm::BasicBlock &DeclareLabel( const Ax::Parser::SToken &Tok )
		{
			return DeclareLabel( Tok.GetPointer(), Tok.cLength );
		}
		llvm::BasicBlock &EmitLabel( const char *pLabelName, size_t cLabelName );
		inline llvm::BasicBlock &EmitLabel( const Ax::String &LabelName )
		{
			return EmitLabel( LabelName.CString(), LabelName.Len() );
		}
		inline llvm::BasicBlock &EmitLabel( const Ax::Parser::SToken &Tok )
		{
			return EmitLabel( Tok.GetPointer(), Tok.cLength );
		}

		llvm::Value *GetDefaultConstant( EBuiltinType BTy );
		void EmitConstruct( llvm::Value *pStorePtr, const STypeRef &Type );
		void EmitDestruct( llvm::Value *pStorePtr, const STypeRef &Type );

		llvm::Value *EmitCast( ECast CastOp, EBuiltinType DstType, llvm::Value *pSrcVal );
		llvm::StoreInst *EmitStore( const SSymbol &DstVar, CExpression &Expr );

		llvm::Value *EmitDefaultInstance( const STypeRef &Type );
		
		inline SInternalFunctions &InternalFuncs()
		{
			return m_IntFuncs;
		}
		llvm::CallInst *EmitAutoprintCall( llvm::Value *pParm );

		llvm::Value *EmitCondition( CExpression &Expr );

		// Enter a clean-up scope
		//
		// return: True if scope is top-level; false if not
		bool EnterScope();
		// Leave the current scope
		void LeaveScope();
		// Emit clean-up code for the current scope
		void CleanScope( llvm::Value *pIgnoreVal = nullptr );
		// Emit clean-up code for the entire function
		void CleanTopLevel( llvm::Value *pIgnoreVal = nullptr );
		// Add a clean-up call to the scope
		void AddCleanCall( llvm::Function *pFunc, llvm::Value *pArg );
		// Remove a clean-up call
		void RemoveCleanCall( llvm::Value *pArg );

		unsigned GetStringId();
		unsigned GetTypeId();
		llvm::Type *GetRTTIType();

		llvm::FunctionType *GetObjInitFnTy();
		llvm::FunctionType *GetObjFiniFnTy();
		llvm::FunctionType *GetObjCopyFnTy();
		llvm::FunctionType *GetObjMoveFnTy();

		void RegisterUDT( STypeInfo &UDT );

		void EmitModuleInfo();

	private:
		llvm::LLVMContext &			m_Context;
		llvm::PassRegistry *		m_pPassReg;
		llvm::Module *				m_pModule;
		llvm::IRBuilder<>			m_IRBuilder;
		llvm::Function *			m_pEntryFunc;
		llvm::Function *			m_pCurrentFunc;
		llvm::BasicBlock *			m_pCurrentBlock;
		llvm::legacy::PassManager *	m_pPM;
		llvm::legacy::FunctionPassManager *m_pFPM;
		const llvm::Target *		m_pTarget;
		llvm::TargetMachine *		m_pTargetMachine;
		llvm::tool_output_file *	m_pObjFile;
		llvm::tool_output_file *	m_pAsmFile;
		SInternalFunctions			m_IntFuncs;
		Ax::TArray<SCleanupScope>	m_CleanScopes;
		bool						m_bOptimize;
		bool						m_bLabelDebugOut;
		unsigned					m_uStringId;
		unsigned					m_uTypeId;
		llvm::Type *				m_pRTTITy;
		llvm::FunctionType *		m_pObjInitFTy;
		llvm::FunctionType *		m_pObjFiniFTy;
		llvm::FunctionType *		m_pObjCopyFTy;
		llvm::FunctionType *		m_pObjMoveFTy;
		Ax::TArray< STypeInfo * >	m_UserTypes;

		MCodeGen();
		~MCodeGen();

		AX_DELETE_COPYFUNCS(MCodeGen);
	};
	static Ax::TManager< MCodeGen >	CG;

}}
