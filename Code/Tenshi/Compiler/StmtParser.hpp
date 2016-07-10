#pragma once

#include "Parser.hpp"
#include "Node.hpp"
#include "Symbol.hpp"
#include "Operator.hpp"
#include "TypeInformation.hpp"

namespace Tenshi { namespace Compiler {

	enum class ELoopFlow
	{
		Break,
		Continue
	};
	enum class EVarScope
	{
		Local,
		Global
	};

	//
	//	Block Statement
	//	===============
	//	Base class for statements that have a sequence of additional statements
	//	(such as WHILE/ENDWHILE or FOR/NEXT)
	//
	class CBlockStatement: public CStatement
	{
	public:
		inline CBlockStatement( EStmtSeqType SeqType, EStmtType Type, const SToken &Tok, CParser &Parser )
		: CStatement( Type, Tok, Parser )
		, m_Stmts()
		{
			m_Stmts.SetType( SeqType );
		}
		virtual ~CBlockStatement() {}

		virtual void SetSequence( CStatementSequence &Seq ) AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	protected:
		CStatementSequence			m_Stmts;

		AX_DELETE_COPYFUNCS(CBlockStatement);
	};
	
	/*
	===========================================================================

		DERIVED STATEMENTS

	===========================================================================
	*/

	//
	//	Function Call Statement
	//	=======================
	//	Represents the action of invoking a function, without checking its
	//	return value.
	//
	//	# <fncallstmt> ::= <function-name> "(" <parameter-expr-list> ")"
	//	#                | <function-name> <parameter-expr-list>
	//	#                ;
	//
	class CFunctionCallStmt: public CStatement
	{
	public:
		CFunctionCallStmt( const SToken &Tok, CParser &Parser );
		virtual ~CFunctionCallStmt() {}

		bool Parse();
		void Parsed( CExpression &Expr );

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		CExpression *				m_pExpr;
		struct
		{
			bool					bIsAutoprint;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CFunctionCallStmt);
	};

	//
	//	Label Declaration Statement
	//	===========================
	//	Represents the declaration of a label.
	//
	//	This is its own statement because the specific point at which the label
	//	has been declared is relevant.
	//
	//	# <labeldeclstmt> ::= <label-name> ":"
	//	#                   ;
	//
	class CLabelDeclStmt: public CStatement
	{
	public:
		CLabelDeclStmt( const SToken &Tok, CParser &Parser );
		virtual ~CLabelDeclStmt() {}

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		struct
		{
			SSymbol *				pLabelSym;
		}							m_Semanted;
	};

	//
	//	Go-To/Go-Sub Statement
	//	======================
	//	Represents the action of jumping to another location in code,
	//	potentially with the address of the next instruction stored on the stack
	//	(a call, for GOSUB).
	//
	//	# <gotostmt> ::= "GOTO" <label-name>
	//	#              | "GOSUB" <label-name>
	//	#              ;
	//
	class CGotoStmt: public CStatement
	{
	public:
		CGotoStmt( const SToken &Tok, CParser &Parser );
		virtual ~CGotoStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		bool						m_bIsGosub;
		const SToken *				m_pLabelTok;
		struct
		{
			const SSymbol *			pLabelSym;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CGotoStmt);
	};
	//
	//	Return Statement
	//	================
	//	Returns to the point after the previous GOSUB call.
	//
	//	NOTE: This cannot be used to return from a function.
	//
	//	# <returnstmt> ::= "RETURN"
	//	#                ;
	//
	class CReturnStmt: public CStatement
	{
	public:
		CReturnStmt( const SToken &Tok, CParser &Parser );
		virtual ~CReturnStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool CodeGen() AX_OVERRIDE;

	private:
		AX_DELETE_COPYFUNCS(CReturnStmt);
	};
	//
	//	Loop-Flow Statement
	//	===================
	//	Represents an EXIT LOOP or REPEAT LOOP action. The equivalents in C are
	//	`break` and `continue`.
	//
	//	# <loopflowstmt> ::= "EXIT"
	//	#                  | "EXIT LOOP"
	//	#                  | "REPEAT LOOP"
	//	#                  ;
	//
	class CLoopFlowStmt: public CStatement
	{
	public:
		CLoopFlowStmt( const SToken &Tok, CParser &Parser );
		virtual ~CLoopFlowStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool CodeGen() AX_OVERRIDE;

	private:
		ELoopFlow					m_Type;

		AX_DELETE_COPYFUNCS(CLoopFlowStmt);
	};
	//
	//	Do-Loop Statement
	//	=================
	//	Represents an infinite loop.
	//
	//	Do-Loops are always "safe" in that they call SYNC regardless of whether
	//	safety code is enabled or not.
	//
	//	# <doloopstmt> ::= "DO" <loop-stmt-sequence> "LOOP"
	//	#                ;
	//
	class CDoLoopStmt: public CBlockStatement
	{
	public:
		CDoLoopStmt( const SToken &Tok, CParser &Parser );
		virtual ~CDoLoopStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		const SToken *				m_pLoopToken;

		AX_DELETE_COPYFUNCS(CDoLoopStmt);
	};
	//
	//	While-Loop Statement
	//	====================
	//	Represents a conditional loop, which evaluates the condition before each
	//	iteration.
	//
	//	While-Loops will have a SYNC call generated at the end of the loop's
	//	iteration only if safety code is enabled.
	//
	//	# <whileloopstmt> ::= "WHILE" <condition-expr> <loop-stmt-sequence> "ENDWHILE"
	//	#                   ;
	//
	class CWhileLoopStmt: public CBlockStatement
	{
	public:
		CWhileLoopStmt( const SToken &Tok, CParser &Parser );
		virtual ~CWhileLoopStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		const SToken *				m_pEndwhileToken;
		CExpression *				m_pCondition;

		AX_DELETE_COPYFUNCS(CWhileLoopStmt);
	};
	//
	//	Repeat-Loop Statement
	//	=====================
	//	Represents a conditional loop, which evaluates the condition after each
	//	iteration.
	//
	//	Repeat-Loops will have a SYNC call generated at the end of the loop's
	//	iteration only if safety code is enabled.
	//
	//	# <repeatloopstmt> ::= "REPEAT" <loop-stmt-sequence> "UNTIL" <condition-expr>
	//	#                    ;
	//
	class CRepeatLoopStmt: public CBlockStatement
	{
	public:
		CRepeatLoopStmt( const SToken &Tok, CParser &Parser );
		virtual ~CRepeatLoopStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		const SToken *				m_pUntilToken;
		CExpression *				m_pCondition;

		AX_DELETE_COPYFUNCS(CRepeatLoopStmt);
	};
	//
	//	For-Loop Statement
	//	==================
	//	Represents an iteration over a range of values.
	//
	//	For-Loops never have any safety code generated, as the end of the loop
	//	is "guaranteed" (except for hackish~ user intervention).
	//
	//	For-Loops come in two forms:
	//
	//		FOR-TO: A ranged iteration from some starting value to (and
	//		-       including) an ending value. FOR i = 1 TO 3 : PRINT i : NEXT
	//		-       would print "1", "2", and "3".
	//
	//		FOR-UNTIL: A ranged iteration from some starting value to (but
	//		-       excluding) an ending value. FOR i = 1 UNTIL 3 : PRINT i :
	//		-       NEXT would print "1", and "2".
	//
	//	Each type of FOR-LOOP can also include an optional STEP clause, which
	//	states how far to increment the passed-variable after each iteration of
	//	the loop.
	//
	//	# <forloopstmt> ::= "FOR" <varname> "=" <expr> ( "TO" | "UNTIL" ) <expr>
	//	#                   ( "STEP" <expr> )?
	//	#                       <loop-stmt-sequence>
	//	#                   "NEXT" <varname>?
	//	#                 ;
	//
	class CForLoopStmt: public CBlockStatement
	{
	public:
		CForLoopStmt( const SToken &Tok, CParser &Parser );
		virtual ~CForLoopStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		const SToken *				m_pVarToken;
		const SToken *				m_pToUntilToken;
		const SToken *				m_pStepToken;
		const SToken *				m_pNextToken;
		CExpression *				m_pInitExpr;
		CExpression *				m_pCondExpr;
		CExpression *				m_pStepExpr;

		struct
		{
			SSymbol *				pVar;
			bool					bOwnsVar;
			ECast					InitCast;
			ECast					CondCast;
			ECast					StepCast;
		}							m_Semant;

		bool ParseInit();
		bool ParseCond();
		bool ParseStep();

		AX_DELETE_COPYFUNCS(CForLoopStmt);
	};
	//
	//	Select Statement
	//	================
	//	Represents the equivalent of a C switch-block.
	//
	//	TODO: Explain better.
	//
	//	# <selectstmt> ::= "SELECT" <expr> <casestmt>+ "ENDSELECT"
	//	#                ;
	//
	class CSelectStmt: public CBlockStatement
	{
	public:
		CSelectStmt( const SToken &Tok, CParser &Parser );
		virtual ~CSelectStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		const SToken *				m_pCaseDefaultToken;
		const SToken *				m_pEndselectToken;
		CExpression *				m_pSelectExpr;

		AX_DELETE_COPYFUNCS(CSelectStmt);
	};
	//
	//	Selection Case Statement
	//	========================
	//	Represents one clause of a SELECT statement.
	//
	//	TODO: Explain better.
	//
	//	# <casestmt> ::= "CASE" <const-expr> <case-stmt-sequence> "ENDCASE"
	//	#              | "CASE DEFAULT" <case-stmt-sequence> "ENDCASE"
	//	#              ;
	//
	class CSelectCaseStmt: public CBlockStatement
	{
	public:
		CSelectCaseStmt( const SToken &Tok, CParser &Parser );
		virtual ~CSelectCaseStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		bool						m_bIsDefault;
		CExpression *				m_pExpr;

		AX_DELETE_COPYFUNCS(CSelectCaseStmt);
	};
	//
	//	Case Fallthrough Statement
	//	==========================
	//	Represents a "fallthrough" from one case statement to the next.
	//
	//	Case statements (select-block) do not allow one case to fall into the
	//	next. For example:
	//
	//		SELECT x
	//			CASE 1 : ENDCASE
	//			CASE 2 : PRINT "2" : ENDCASE
	//		ENDSELECT
	//
	//	Would not print "2" if x is 1, as expected. To enable that scenario, a
	//	fallthrough statement must be used:
	//
	//		SELECT x
	//			CASE 1 : FALLTHROUGH : ENDCASE
	//			CASE 2 : PRINT "1 or 2" : ENDCASE
	//		ENDSELECT
	//
	//	Would print "1 or 2" if x is either 1 or 2.
	//
	//	# <fallthroughstmt> ::= "FALLTHROUGH"
	//	#                     ;
	//
	class CCaseFallthroughStmt: public CStatement
	{
	public:
		CCaseFallthroughStmt( const SToken &Tok, CParser &Parser );
		virtual ~CCaseFallthroughStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool CodeGen() AX_OVERRIDE;

	private:
		AX_DELETE_COPYFUNCS(CCaseFallthroughStmt);
	};
	//
	//	If Statement
	//	============
	//	Represents a conditional-flow action (if statement).
	//
	//	TODO: Explain better.
	//
	//	TODO: Grammar.
	//
	class CIfStmt: public CBlockStatement
	{
	public:
		CIfStmt( const SToken &Tok, CParser &Parser );
		virtual ~CIfStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		bool						m_bIsElse;
		CExpression *				m_pCondition;
		Ax::TArray< CIfStmt * >		m_ElseStmts;
		struct
		{
			llvm::BasicBlock *		pDestBlock;
		}							m_CodeGen;

		AX_DELETE_COPYFUNCS(CIfStmt);
	};

	//
	//	Variable Declaration Statement
	//	==============================
	//	Represents a variable declaration.
	//
	//	The type of the variable is inferred if the type is not explicitly
	//	given. Type inferrence is performed as follows:
	//
	//	- If the variable has a "#" or "$" in its name then it is a string.
	//	- Otherwise, if an initialization expression is given then the type of
	//	  the variable is the same as the type of the expression.
	//	- If neither of the above, then the variable is a pointer-sized integer.
	//
	//	Variables can be declared either in the local scope or in global scope.
	//	If a scope is omitted, then the local scope is assumed.
	//
	//	The local scope is the most-recently introduced scope. e.g., as through
	//	a function definition.
	//
	//	Variables declared with global scope inside of functions are equivalent
	//	to a "static" variable within a function definition in C++. If the
	//	variable is trivially constructible then it will be initialized prior to
	//	any other function running, regardless of whether the function is ever
	//	called. Otherwise, the variable is non-trivially constructible, and will
	//	only be initialized upon the function's first call, in any thread, and
	//	in a thread-safe manner.
	//
	//	Variables can be declared with a default/initialization value. If no
	//	value is given then the variable will be zeroed if its type has no
	//	specific construction-policy; otherwise the constructor of the type will
	//	be invoked. (e.g., a matrix type might be initialized to the identity.)
	//
	//	# <vardeclstmt> ::= ( "LOCAL" | "GLOBAL" )? <var-name>
	//	#                   ( "AS" <type-name> )? ( "=" <expr> )?
	//	#                 | ( "LOCAL" | "GLOBAL" )? <type-name> <var-name-seq>
	//	#                 ;
	//	# <var-name-seq> ::= <var-name-item>
	//	#                  | <var-name-seq> "," <var-name-item>
	//	#                  ;
	//	# <var-name-item> ::= <var-name> ( "=" <expr> )?
	//	#                   ;
	//
	class CVarDeclStmt: public CStatement
	{
	public:
		CVarDeclStmt( const SToken &Tok, CParser &Parser );
		virtual ~CVarDeclStmt();

		bool Parse();
		bool Parse( const SToken &NameTok );

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		EVarScope					m_DeclScope;
		const SToken *				m_pVarTok;
		CExpressionList *			m_pDimExprs;
		CTypeDecl *					m_pType;
		CExpression *				m_pDefExpr;
		SSymbol *					m_pVarSym;
		SMemberInfo					m_Info;
		STypeRef *					m_pTypeRef;

		struct
		{
			ECast					CastOp;
		}							m_Semanted;

		bool ParseType();

		AX_DELETE_COPYFUNCS(CVarDeclStmt);
	};
	//
	//	Variable Assignment Statement
	//	=============================
	//	Represents the action of assigning a value to a variable.
	//
	//	The assignment can be a compound-operation. For example, v += x is the
	//	same as v = v + ( x ).
	//
	//	# <varassignstmt> ::= <var-name> <assign-op> <expr>
	//	#                   ;
	//
	class CVarAssignStmt: public CStatement
	{
	public:
		CVarAssignStmt( const SToken &Tok, CParser &Parser );
		virtual ~CVarAssignStmt() {}

		bool Parse( CExpression &VarExpr );

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		const SToken *				m_pOpTok;
		EBuiltinOp					m_CompoundOp;
		CExpression *				m_pVarExpr;
		CExpression *				m_pValueExpr;
		struct
		{
			ECast					CastOp;
			EBuiltinType			CastType;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CVarAssignStmt);
	};
	//
	//	Exit-Function Statement
	//	=======================
	//	Represents the action of returning from a function (potentially with a
	//	value).
	//
	//	If EXITFUNCTION is invoked without a parameter in a function that
	//	returns a value then a default constructed value is returned in
	//	non-strict mode. In strict mode, however, this is an error.
	//
	//	If EXITFUNCTION is invoked with a parameter in a fnction that does not
	//	return a value then the type of the parameter must be void. Any other
	//	type is an error. (The expression that results in void will still be
	//	evaluated.)
	//
	//	The only valid place to call EXITFUNCTION is within a function. Calling
	//	it within the global scope is an error.
	//
	//	ENDFUNCTION has the same semantics as EXITFUNCTION, as outlined above,
	//	but with different surrounding syntax rules and implications. (That is
	//	because ENDFUNCTION is used to finalize the function's definition.)
	//
	//	# <exitfnstmt> ::= "EXITFUNCTION" <expr>?
	//	#                ;
	//
	class CExitFunctionStmt: public CStatement
	{
	public:
		CExitFunctionStmt( const SToken &Tok, CParser &Parser );
		virtual ~CExitFunctionStmt() {}

		bool Parse();

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual bool CodeGen() AX_OVERRIDE;

	private:
		CExpression *				m_pExpr;
		struct
		{
			ECast					CastOp;
			const STypeRef *		pReturnType;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CExitFunctionStmt);
	};

}}