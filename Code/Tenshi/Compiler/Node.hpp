#pragma once

#include <Core/Types.hpp>
#include <Collections/List.hpp>
#include <Collections/Array.hpp>
#include "Lexer.hpp"
#include "Operator.hpp"
#include "BuiltinType.hpp"

namespace Tenshi { namespace Compiler {

	struct STypeInfo;

	class CParser;

	struct SSymbol;
	class CScope;

	class CExpression;
	class CStatement;

	class CTypeDecl;
	struct STypeRef;

	enum class EAccess
	{
		ReadOnly,
		WriteOnly,
		ReadWrite
	};

	enum class EStmtSeqType
	{
		None,

		Program,

		Function,
		SelectBlock,
		CaseBlock,
		LoopBlock
	};
	enum class EStmtType
	{
		None,

		LabelDecl,
		VarDecl,

		GotoStmt,
		ExitStmt,
		FallthroughStmt,
		ExitFunctionStmt,
		ReturnStmt,

		AssignVar,
		InvokeFunction,

		IfBlock,
		SelectBlock,
		CaseStmt,

		DoLoopBlock,
		WhileBlock,
		RepeatBlock,
		ForNextBlock,
	};
	enum class EExprType
	{
		None,

		ExpressionList,
		Literal,
		Name,
		Member,
		ArraySubscript,
		FunctionCall,
		UnaryOp,
		BinaryOp
	};

	struct SValue
	{
		enum EKind: Ax::uint8
		{
			kLValue,
			kRValue
		};
		EKind						Kind;
		bool						bIsVolatile:1;
		llvm::Value *				pLLVMValue;

		inline SValue( EKind InKind = kRValue, llvm::Value *pInValue = nullptr )
		: Kind( InKind )
		, bIsVolatile( false )
		, pLLVMValue( pInValue )
		{
		}
		inline SValue( llvm::Value *pInValue )
		: Kind( kRValue )
		, bIsVolatile( false )
		, pLLVMValue( pInValue )
		{
		}
		inline SValue( llvm::Value *pInValue, bool bInIsVolatile )
		: Kind( kLValue )
		, bIsVolatile( bInIsVolatile )
		, pLLVMValue( pInValue )
		{
		}
		inline SValue( decltype(nullptr) )
		: Kind( kRValue )
		, bIsVolatile( false )
		, pLLVMValue( nullptr )
		{
		}

		inline bool operator!() const
		{
			return !pLLVMValue;
		}
		inline operator bool() const
		{
			return pLLVMValue != nullptr;
		}
		inline bool operator==( EKind InKind ) const
		{
			return Kind == InKind;
		}
		inline bool operator!=( EKind InKind ) const
		{
			return Kind != InKind;
		}
		inline operator EKind() const
		{
			return Kind;
		}

		inline llvm::Value *operator->()
		{
			AX_ASSERT_NOT_NULL( pLLVMValue );
			return pLLVMValue;
		}

		inline bool IsAddress() const
		{
			return Kind == kLValue;
		}
		inline bool IsImmediate() const
		{
			return Kind == kRValue;
		}
		inline bool IsVolatile() const
		{
			return bIsVolatile;
		}

		llvm::Value *Address();
		llvm::Value *Load();
		void Store( SValue FromVal );
	};

	//
	//	Type Particle: Used by CTypeDecl during parsing to determine how a type
	//	is made up
	//
	struct STypeParticle
	{
		// What this particle represents
		enum ETypePart
		{
			kTypeName = 0,

			kPointer,
			kReference,
			kArray,
			kLinkedList,
			kBinaryTree,
			kConstQualifier,
			kMutableQualifier,
			kVolatileQualifier
		};
		union
		{
			// One of the ETypePart members above
			Ax::uintptr				uValue;
			// Or the pointer if uValue > 4096
			const SToken *			pToken;
		};

		inline bool IsTypeName() const
		{
			return uValue == 0 || uValue > 4096;
		}
		inline bool IsPointer() const
		{
			return uValue == kPointer;
		}
		inline bool IsReference() const
		{
			return uValue == kReference;
		}
		inline bool IsArray() const
		{
			return uValue == kArray;
		}
		inline bool IsLinkedList() const
		{
			return uValue == kLinkedList;
		}
		inline bool IsBinaryTree() const
		{
			return uValue == kBinaryTree;
		}
		inline bool IsConstQualifier() const
		{
			return uValue == kConstQualifier;
		}
		inline bool IsMutableQualifier() const
		{
			return uValue == kMutableQualifier;
		}
		inline bool IsVolatileQualifier() const
		{
			return uValue == kVolatileQualifier;
		}

		inline ETypePart GetType() const
		{
			return IsTypeName() ? kTypeName : ( ETypePart )uValue;
		}

		inline const SToken *TokenPointer() const
		{
			return IsTypeName() ? pToken : nullptr;
		}

		inline const SToken *operator->() const
		{
			return TokenPointer();
		}
		
		inline void SetTypeName( const SToken *pTypeTok )
		{
			pToken = pTypeTok;
		}
		inline void SetPointer()
		{
			uValue = kPointer;
		}
		inline void SetReference()
		{
			uValue = kReference;
		}
		inline void SetArray()
		{
			uValue = kArray;
		}
		inline void SetLinkedList()
		{
			uValue = kLinkedList;
		}
		inline void SetBinaryTree()
		{
			uValue = kBinaryTree;
		}
		inline void SetConstQualifier()
		{
			uValue = kConstQualifier;
		}
		inline void SetMutableQualifier()
		{
			uValue = kMutableQualifier;
		}
		inline void SetVolatileQualifier()
		{
			uValue = kVolatileQualifier;
		}
	};
	//
	//	Parsed declaration of a type
	//	Yields a STypeRef after Semant().
	//
	class CTypeDecl
	{
	public:
		CTypeDecl( const SToken &Tok, CParser &Parser );
		~CTypeDecl();

		bool Parse();
		bool ParseOne();

		Ax::String ToString() const;

		bool Semant( const SToken &NameTok );

		STypeRef &Ref();
		const STypeRef &Ref() const;

	private:
		const SToken &				m_Token;
		CParser &					m_Parser;
		CLexer &					m_Lexer;
		Ax::TArray< STypeParticle >	m_Chain;
		STypeRef *					m_pRef;

		STypeParticle &AllocParticle();
	};
	//
	//	Reference to types as used by the expression parser during the semantic
	//	analysis and later phases.
	//
	struct STypeRef
	{
		// The built-in type (valid if pCustomType is NULL)
		EBuiltinType				BuiltinType;
		// The custom type (valid if BuiltinType == EBuiltinType::UserDefined)
		const STypeInfo *			pCustomType;
		// Sub-type (e.g., if this is an array then an array of what? Integers? Strings? Linked lists of strings?)
		STypeRef *					pRef;
		// Whether this data can be read to/written from
		EAccess						Access;
		// Whether the compiler is free to re-order memory accesses
		bool						bCanReorderMemAccesses;
		// Exact size of this type (if it's an array, or similar object, this will be the size of a pointer)
		Ax::uint32					cBytes;

		// Internal LLVM type
		struct
		{
			bool					bDidTranslate;
			llvm::Type *			pType;
		}							Translated;

		inline STypeRef()
		{
			Reset();
		}
		inline ~STypeRef()
		{
			delete pRef;
			pRef = nullptr;
		}

		inline void Reset()
		{
			BuiltinType = EBuiltinType::Invalid;
			pCustomType = nullptr;
			pRef = nullptr;
			Access = EAccess::ReadWrite;
			bCanReorderMemAccesses = true;
			cBytes = 0;

			Translated.bDidTranslate = false;
			Translated.pType = nullptr;
		}

		inline bool IsArray() const
		{
			return BuiltinType == EBuiltinType::ArrayObject;
		}
		inline bool IsVector() const
		{
			return GetTypeColumns( BuiltinType ) > 0;
		}

		inline bool IsVolatile() const
		{
			return !bCanReorderMemAccesses;
		}

		Ax::String ToString() const;

		llvm::Type *CodeGen();

		llvm::Type *GetValueType() const;
		llvm::Type *GetPointerType() const;

		inline llvm::Type *GetParameterType() const
		{
			if( BuiltinType == EBuiltinType::UserDefined ) {
				return GetPointerType();
			}

			return GetValueType();
		}

		bool IsEmbeddedAtHead( const STypeInfo &UDT ) const;

		static bool Semant( STypeRef &OutRef, const Ax::Parser::SToken &NameTok, CTypeDecl *pDecl );
		static STypeRef *Semant( const Ax::Parser::SToken &NameTok, CTypeDecl *pDecl );

		static ECast Cast( const STypeRef &From, const STypeRef &To );
		// Throws a cast error -- always returns false
		static bool CastError( const SToken &Tok, const STypeRef &From, const STypeRef &To );
	};

	class CStatementSequence
	{
	public:
		CStatementSequence();
		~CStatementSequence();

		void SetType( EStmtSeqType Type );
		EStmtSeqType Type() const;
		bool Is( EStmtSeqType Type ) const;

		void Inherit( const CStatementSequence &OtherSeq );
		bool InFunction() const;
		bool InCase() const;
		bool InLoop() const;

		void SetFunction( SSymbol *pSym );
		void SetScope( CScope *pScope );
		SSymbol *FunctionSym();
		CScope *Scope();
		const SSymbol *FunctionSym() const;
		const CScope *Scope() const;

		CStatement **begin();
		CStatement **end();
		const CStatement *const *begin() const;
		const CStatement *const *end() const;

		CStatement &AddStmt( CStatement &NewStmt );
		template< typename tStmtType >
		inline tStmtType &AddStmt( tStmtType &InNewStmt )
		{
			return static_cast< tStmtType & >( AddStmt( static_cast< CStatement & >( InNewStmt ) ) );
		}
		template< typename tStmtType >
		inline tStmtType &NewStmt( const SToken &Tok, CParser &Parser )
		{
			tStmtType *const pStmt = new tStmtType( Tok, Parser );
			AX_EXPECT_MEMORY( pStmt );
			
			return static_cast< tStmtType & >( AddStmt( static_cast< CStatement & >( *pStmt ) ) );
		}

		Ax::String ToString() const;

		bool Semant();
		bool CodeGen();

	private:
		Ax::TArray< CStatement * >	m_Statements;
		EStmtSeqType				m_Type;
		bool						m_bInFunction;
		bool						m_bInCase;
		bool						m_bInLoop;

		SSymbol *					m_pFunction;
		CScope *					m_pScope;
	};

	class CStatement
	{
	friend class CParser;
	friend class CStatementSequence;
	public:
		virtual ~CStatement();

		inline bool Is( EStmtType InType ) const { return m_Type == InType; }
		EStmtType Type() const;
		const CParser &Parser() const;
		const SToken &Token() const;

		bool IsSequenceSet() const;
		virtual void SetSequence( CStatementSequence &Seq );
		CStatementSequence &GetSequence() const;

		virtual Ax::String ToString() const;

		virtual bool Semant();
		virtual bool CodeGen();

	protected:
		CStatement( EStmtType Type, const SToken &Token, CParser &Parser );

		CLexer &Lexer();
		const CLexer &Lexer() const;

		CParser &Parser();

	private:
		const EStmtType				m_Type;
		const SToken &				m_Token;
		CParser &					m_Parser;
		CStatementSequence *		m_pSequence;

		CStatement( const CStatement & ) AX_DELETE_FUNC;
		CStatement &operator=( const CStatement & ) AX_DELETE_FUNC;
	};

	class CExpression
	{
	friend class CParser;
	public:
		virtual ~CExpression();

		inline bool Is( EExprType InType ) const { return m_Type == InType; }
		EExprType Type() const;
		const CParser &Parser() const;
		const SToken &Token() const;

		void SetParent( CExpression *pParent );
		CExpression *Parent();
		const CExpression *Parent() const;

		virtual Ax::String ToString() const;
		virtual const SSymbol *GetSymbol() const;
		virtual const STypeRef *GetType() const;
		bool IsLValue() const;

		virtual bool Semant();
		virtual SValue CodeGen();

	protected:
		CExpression( EExprType Type, const SToken &Token, CParser &Parser );

		CLexer &Lexer();
		const CLexer &Lexer() const;

		CParser &Parser();

	private:
		const EExprType				m_Type;
		const SToken &				m_Token;
		CParser &					m_Parser;
		CExpression *				m_pParent;

		CExpression( const CExpression & ) AX_DELETE_FUNC;
		CExpression &operator=( const CExpression & ) AX_DELETE_FUNC;
	};

	struct SExpr
	{
		CExpression *pExpr;
		ECast Cast;

		inline SExpr()
		: pExpr( nullptr )
		, Cast( ECast::None )
		{
		}
		inline SExpr( CExpression *pExpr )
		: pExpr( pExpr )
		, Cast( ECast::None )
		{
		}
		inline SExpr( CExpression *pExpr, ECast Cast )
		: pExpr( pExpr )
		, Cast( Cast )
		{
		}
		inline SExpr( const SExpr &x )
		: pExpr( const_cast< CExpression * >( x.pExpr ) )
		, Cast( x.Cast )
		{
		}

		inline operator CExpression *() { return pExpr; }
		inline operator const CExpression *() const { return pExpr; }

		inline CExpression &operator*() { return *pExpr; }
		inline const CExpression &operator*() const { return *pExpr; }

		inline CExpression *operator->() { return pExpr; }
		inline const CExpression *operator->() const { return pExpr; }

		inline SExpr &operator=( CExpression *pOtherExpr )
		{
			pExpr = pOtherExpr;
			Cast = ECast::None;

			return *this;
		}
		inline SExpr &operator=( ECast OtherCast )
		{
			Cast = OtherCast;

			return *this;
		}
		inline SExpr &operator=( const SExpr &x )
		{
			pExpr = const_cast< CExpression * >( x.pExpr );
			Cast = x.Cast;

			return *this;
		}

		inline bool operator!() const
		{
			return !pExpr || Cast == ECast::Invalid;
		}

		inline bool operator==( const CExpression *const pOtherExpr ) const
		{
			return pExpr == pOtherExpr;
		}
		inline bool operator!=( const CExpression *const pOtherExpr ) const
		{
			return pExpr != pOtherExpr;
		}
	};

	//========================================================================//

	inline const SToken &CStatement::Token() const
	{
		return m_Token;
	}
	
	inline EExprType CExpression::Type() const
	{
		return m_Type;
	}
	inline const SToken &CExpression::Token() const
	{
		return m_Token;
	}
	inline const CParser &CExpression::Parser() const
	{
		return m_Parser;
	}

	inline CExpression *CExpression::Parent()
	{
		return m_pParent;
	}
	inline const CExpression *CExpression::Parent() const
	{
		return m_pParent;
	}

	//========================================================================//

	inline STypeRef &CTypeDecl::Ref()
	{
		AX_ASSERT_NOT_NULL( m_pRef );
		return *m_pRef;
	}
	inline const STypeRef &CTypeDecl::Ref() const
	{
		AX_ASSERT_NOT_NULL( m_pRef );
		return *m_pRef;
	}

}}
