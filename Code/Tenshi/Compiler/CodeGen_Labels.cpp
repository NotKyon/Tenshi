#include "_PCH.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	inline String Str( llvm::StringRef r )
	{
		return String( r.data(), ( intptr )r.size() );
	}

	bool MCodeGen::HasCurrentBlock() const
	{
		return m_pCurrentBlock != nullptr;
	}
	llvm::BasicBlock &MCodeGen::CurrentBlock()
	{
		AX_ASSERT_NOT_NULL( m_pCurrentBlock );
		return *m_pCurrentBlock;
	}
	void MCodeGen::SetCurrentBlock( llvm::BasicBlock &Block )
	{
		if( m_bLabelDebugOut ) {
			g_DebugLog += "Setting basic block: \"" + Str( Block.getName() ) + "\"";
		}
		if( m_pCurrentBlock == &Block ) {
			if( m_bLabelDebugOut ) {
				g_DebugLog += "\tBlock is already set";
			}
			return;
		}

		if( Block.getParent() == nullptr && m_pCurrentFunc != nullptr ) {
			if( m_bLabelDebugOut ) {
				g_DebugLog += "\tAdding block to current function (\"" + Str( m_pCurrentFunc->getName() ) + "\")";
			}
			m_pCurrentFunc->getBasicBlockList().push_back( &Block );
		}

		llvm::Function *const pBlockFunc = Block.getParent();

		if( m_pCurrentBlock != nullptr ) {
			const bool bHasTerminator = m_pCurrentBlock->getTerminator() != nullptr;
			if( m_bLabelDebugOut ) {
				g_DebugLog += bHasTerminator ? "\tBlock has terminator" : "\tBlock does NOT have terminator";
			}
			if( !bHasTerminator && m_pCurrentBlock->getParent() == pBlockFunc ) {
				if( m_bLabelDebugOut ) {
					g_DebugLog += "\tAdding branch from previous block (\"" + Str( m_pCurrentBlock->getName() ) + "\") to this block";
				}
				m_IRBuilder.CreateBr( &Block );
			}
		}

		m_pCurrentBlock = &Block;
		if( m_pCurrentFunc != pBlockFunc ) {
			m_pCurrentFunc = pBlockFunc;
		}
		m_IRBuilder.SetInsertPoint( m_pCurrentBlock );
	}

	bool MCodeGen::HasCurrentFunction() const
	{
		return m_pCurrentFunc != nullptr;
	}
	llvm::Function &MCodeGen::CurrentFunction()
	{
		AX_ASSERT_NOT_NULL( m_pCurrentFunc );
		return *m_pCurrentFunc;
	}
	
	llvm::BasicBlock &MCodeGen::DeclareLabel( const char *pLabelName, size_t cLabelName )
	{
		llvm::Function *const pParent = m_pCurrentFunc;
		const llvm::StringRef Name = LLVMStr( pLabelName, cLabelName );

		llvm::BasicBlock *const pBlock = llvm::BasicBlock::Create( m_Context, Name, pParent );
		AX_EXPECT_MEMORY( pBlock );

		return *pBlock;
	}
	llvm::BasicBlock &MCodeGen::EmitLabel( const char *pLabelName, size_t cLabelName )
	{
		llvm::Function *const pParent = m_pCurrentFunc;
		const llvm::StringRef Name = LLVMStr( pLabelName, cLabelName );

		llvm::BasicBlock *const pBlock = llvm::BasicBlock::Create( m_Context, Name, pParent );
		AX_EXPECT_MEMORY( pBlock );

		SetCurrentBlock( *pBlock );

		return *pBlock;
	}

	// Enter a loop
	bool MCodeGen::EnterLoop( llvm::BasicBlock *pBreakLoop, llvm::BasicBlock *pContinueLoop )
	{
		SLoopPoints points;

		points.pBreakLoop = pBreakLoop;
		points.pContinueLoop = pContinueLoop;

		return m_LoopPoints.Append( points );
	}
	// Leave a loop
	void MCodeGen::LeaveLoop()
	{
		AX_ASSERT_MSG( m_LoopPoints.IsEmpty() == false, "Not in a loop!" );

		m_LoopPoints.RemoveLast();
	}
	// Break from a loop
	void MCodeGen::BreakLoop()
	{
		AX_ASSERT_MSG( m_LoopPoints.IsEmpty() == false, "Not in a loop!" );

		m_IRBuilder.CreateBr( m_LoopPoints.Last().pBreakLoop );
	}
	// Continue a loop
	void MCodeGen::ContinueLoop()
	{
		AX_ASSERT_MSG( m_LoopPoints.IsEmpty() == false, "Not in a loop!" );

		m_IRBuilder.CreateBr( m_LoopPoints.Last().pContinueLoop );
	}

}}
