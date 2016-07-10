#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <Collections/Dictionary.hpp>
#include <Collections/List.hpp>
#include <Collections/Array.hpp>

#include <Core/Assert.hpp>
#include <Core/Logger.hpp>
#include <Core/Manager.hpp>
#include <Core/ScopeGuard.hpp>
#include <Core/String.hpp>
#include <Core/Types.hpp>
#include <Core/TypeTraits.hpp>

#include <Platform/Platform.hpp>
#include <Platform/MiscTricks.hpp>

#include <System/AppPath.hpp>
#include <System/FileSystem.hpp>
#include <System/HighPerformanceClock.hpp>
#include <System/TimeConversion.hpp>
#include <System/Timer.hpp>
#include <System/UUID.hpp>

#include <Parser/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <Parser/Source.hpp>
#include <Parser/Token.hpp>

#pragma warning(push)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4800)
#pragma warning(disable:4996)

#include <llvm/ADT/Triple.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/CodeGen/LinkAllAsmWriterComponents.h>
#include <llvm/CodeGen/LinkAllCodegenComponents.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/SubtargetFeature.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetSubtargetInfo.h>
#include <llvm/Transforms/Scalar.h>

template class llvm::IRBuilder<>;

#pragma warning(pop)
