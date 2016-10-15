cmake_minimum_required(VERSION 3.4.1)
project(Tenshi)

set_property(GLOBAL PROPERTY CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX Dbg)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Find the libraries that correspond to the LLVM components
# that we wish to use
#llvm_map_components_to_libnames(llvm_libs support core irreader native nativecodegen object option passes target asmprinter arm x86)
llvm_map_components_to_libnames(llvm_libs support core cppbackend cppbackendcodegen armcodegen native)

# The following is probably what we want on more full LLVM systems
# llvm_map_components_to_libnames(llvm_libs support xcorecodegen core bpfcodegen hexagoncodegen mipscodegen msp430codegen nvptxcodegen powerpccodegen sparccodegen systemzcodegen cppbackend cppbackendcodegen armcodegen amdgpucodegen aarch64codegen native asmprinter systemz)

# ============================================================================ #

set(AXMODS_DIR "Code/Tenshi/Modules")

function(prepend var prefix)
	set(result "")
	foreach(arg ${ARGN})
		list(APPEND result "${prefix}/${arg}")
	endforeach()
	set(${var} "${result}" PARENT_SCOPE)
endfunction()

function(add_tenshi_api moduleName)
	set(files "")
	prepend(files "${AXMODS_DIR}/${moduleName}" ${ARGN})

	add_library("${moduleName}" SHARED ${files})
	set_target_properties("${moduleName}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "plugins-core")
	set_target_properties("${moduleName}" PROPERTIES PREFIX "")
	set_target_properties("${moduleName}" PROPERTIES DEBUG_SUFFIX "Dbg")
	set_target_properties("${moduleName}" PROPERTIES CMAKE_SHARED_LIBRARY_PREFIX "")
	set_target_properties("${moduleName}" PROPERTIES CMAKE_STATIC_LIBRARY_PREFIX "")

	file(COPY "${AXMODS_DIR}/${moduleName}/${moduleName}.commands" DESTINATION "plugins-core")

	target_link_libraries("${moduleName}"
		${ax_libs}
		${llvm_libs}
	)
endfunction()

#==============================================================================#

set_property(DIRECTORY APPEND PROPERTY
  COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG>
)

set(AXLIBS_DIR "Code/Libs")

include_directories(${AXLIBS_DIR})

set(AXLIBS_ALLOCDIR "${AXLIBS_DIR}/Allocation")
set(AXLIBS_ASYNCDIR "${AXLIBS_DIR}/Async")
set(AXLIBS_COLLDIR "${AXLIBS_DIR}/Collections")
set(AXLIBS_CONDIR "${AXLIBS_DIR}/Console")
set(AXLIBS_COREDIR "${AXLIBS_DIR}/Core")
set(AXLIBS_INPUTDIR "${AXLIBS_DIR}/Input")
set(AXLIBS_MATHDIR "${AXLIBS_DIR}/Math")
set(AXLIBS_PARSERDIR "${AXLIBS_DIR}/Parser")
set(AXLIBS_PLATDIR "${AXLIBS_DIR}/Platform")
set(AXLIBS_SYSDIR "${AXLIBS_DIR}/System")
set(AXLIBS_WINDIR "${AXLIBS_DIR}/Window")

set(ax_libs
	AxAllocation
	AxAsync
	AxCollections
	AxConsole
	AxCore
	AxInput
	AxMath
	AxParser
	AxPlatform
	AxSystem
	AxWindow
)

add_library(AxAllocation
	${AXLIBS_ALLOCDIR}/AxAllocation.cpp
	${AXLIBS_ALLOCDIR}/Allocator.cpp
	${AXLIBS_ALLOCDIR}/Allocator.hpp
	${AXLIBS_ALLOCDIR}/New.hpp
	${AXLIBS_ALLOCDIR}/ReadMe.txt
)

add_library(AxAsync
	${AXLIBS_ASYNCDIR}/AxAsync.cpp
	${AXLIBS_ASYNCDIR}/Detail/Atomics_GCC.hpp
	${AXLIBS_ASYNCDIR}/Detail/Atomics_MSVC.hpp
	${AXLIBS_ASYNCDIR}/Detail/MemoryOrder.hpp
	${AXLIBS_ASYNCDIR}/Detail/Threading.hpp
	${AXLIBS_ASYNCDIR}/Detail/Threading_PThread.hpp
	${AXLIBS_ASYNCDIR}/Detail/Threading_Windows.hpp
	${AXLIBS_ASYNCDIR}/Schedulers/ExecStats.cpp
	${AXLIBS_ASYNCDIR}/Schedulers/ExecStats.hpp
	${AXLIBS_ASYNCDIR}/Schedulers/Notify.cpp
	${AXLIBS_ASYNCDIR}/Schedulers/Notify.hpp
	${AXLIBS_ASYNCDIR}/Schedulers/Profiling.cpp
	${AXLIBS_ASYNCDIR}/Schedulers/Profiling.hpp
	${AXLIBS_ASYNCDIR}/Schedulers/Scheduler04.cpp
	${AXLIBS_ASYNCDIR}/Schedulers/Scheduler04.hpp
	${AXLIBS_ASYNCDIR}/Schedulers/SchedulerMutex.hpp
	${AXLIBS_ASYNCDIR}/Schedulers/SchedulerSemaphore.hpp
	${AXLIBS_ASYNCDIR}/Atomic.hpp
	${AXLIBS_ASYNCDIR}/Barrier.hpp
	${AXLIBS_ASYNCDIR}/CPU.cpp
	${AXLIBS_ASYNCDIR}/CPU.hpp
	${AXLIBS_ASYNCDIR}/Mutex.hpp
	${AXLIBS_ASYNCDIR}/QuickMutex.hpp
	${AXLIBS_ASYNCDIR}/QuickSemaphore.hpp
	${AXLIBS_ASYNCDIR}/ReadMe.txt
	${AXLIBS_ASYNCDIR}/RWLock.hpp
	${AXLIBS_ASYNCDIR}/Scheduler.hpp
	${AXLIBS_ASYNCDIR}/Semaphore.hpp
	${AXLIBS_ASYNCDIR}/Signal.hpp
	${AXLIBS_ASYNCDIR}/Thread.cpp
	${AXLIBS_ASYNCDIR}/Thread.hpp
)

add_library(AxCollections
	${AXLIBS_COLLDIR}/AxCollections.cpp
	${AXLIBS_COLLDIR}/Array.hpp
	${AXLIBS_COLLDIR}/Dictionary.hpp
	${AXLIBS_COLLDIR}/List.hpp
	${AXLIBS_COLLDIR}/Pair.hpp
	${AXLIBS_COLLDIR}/SparseArray.hpp
)

add_library(AxConsole
	${AXLIBS_CONDIR}/AxConsole.cpp
	${AXLIBS_CONDIR}/Config.cpp
	${AXLIBS_CONDIR}/Config.hpp
	${AXLIBS_CONDIR}/Var.cpp
	${AXLIBS_CONDIR}/Var.hpp
)

add_library(AxCore
	${AXLIBS_COREDIR}/AxCore.cpp
	${AXLIBS_COREDIR}/Assert.cpp
	${AXLIBS_COREDIR}/Assert.hpp
	${AXLIBS_COREDIR}/Logger.cpp
	${AXLIBS_COREDIR}/Logger.hpp
	${AXLIBS_COREDIR}/Manager.hpp
	${AXLIBS_COREDIR}/ScopeGuard.hpp
	${AXLIBS_COREDIR}/String.cpp
	${AXLIBS_COREDIR}/String.hpp
	${AXLIBS_COREDIR}/Types.hpp
	${AXLIBS_COREDIR}/TypeTraits.hpp
)

add_library(AxInput
	${AXLIBS_INPUTDIR}/AxInput.cpp
	${AXLIBS_INPUTDIR}/Key.hpp
)

add_library(AxMath
	${AXLIBS_MATHDIR}/AxMath.cpp
	${AXLIBS_MATHDIR}/Types/IntVector2.hpp
	${AXLIBS_MATHDIR}/Types/Rect.hpp
	${AXLIBS_MATHDIR}/Basic.hpp
	${AXLIBS_MATHDIR}/Bits.hpp
	${AXLIBS_MATHDIR}/Const.hpp
	${AXLIBS_MATHDIR}/HalfFloat.hpp
	${AXLIBS_MATHDIR}/Intrinsics.hpp
	${AXLIBS_MATHDIR}/Math.hpp
	${AXLIBS_MATHDIR}/Types.hpp
	${AXLIBS_MATHDIR}/Vector.hpp
)

add_library(AxParser
	${AXLIBS_PARSERDIR}/AxParser.cpp
	${AXLIBS_PARSERDIR}/Lexer.cpp
	${AXLIBS_PARSERDIR}/Lexer.hpp
	${AXLIBS_PARSERDIR}/Parser.cpp
	${AXLIBS_PARSERDIR}/Parser.hpp
	${AXLIBS_PARSERDIR}/Source.cpp
	${AXLIBS_PARSERDIR}/Source.hpp
	${AXLIBS_PARSERDIR}/Token.cpp
	${AXLIBS_PARSERDIR}/Token.hpp
)

add_library(AxPlatform
	${AXLIBS_PLATDIR}/AxPlatform.cpp
	${AXLIBS_PLATDIR}/BuildConf.hpp
	${AXLIBS_PLATDIR}/CxxSupport.hpp
	${AXLIBS_PLATDIR}/MiscTricks.hpp
	${AXLIBS_PLATDIR}/Platform.hpp
	${AXLIBS_PLATDIR}/StaticAnalysis.hpp
)

add_library(AxSystem
	${AXLIBS_SYSDIR}/AxSystem.cpp
	${AXLIBS_SYSDIR}/Detail/vc_dirent.h
	${AXLIBS_SYSDIR}/AppPath.cpp
	${AXLIBS_SYSDIR}/AppPath.hpp
	${AXLIBS_SYSDIR}/FileSystem.cpp
	${AXLIBS_SYSDIR}/FileSystem.hpp
	${AXLIBS_SYSDIR}/HighPerformanceClock.cpp
	${AXLIBS_SYSDIR}/HighPerformanceClock.hpp
	${AXLIBS_SYSDIR}/TimeConversion.hpp
	${AXLIBS_SYSDIR}/Timer.hpp
	${AXLIBS_SYSDIR}/UUID.cpp
	${AXLIBS_SYSDIR}/UUID.hpp
)

add_library(AxWindow
	${AXLIBS_WINDIR}/AxWindow.cpp
	${AXLIBS_WINDIR}/Application.cpp
	${AXLIBS_WINDIR}/Application.hpp
	${AXLIBS_WINDIR}/Monitor.cpp
	${AXLIBS_WINDIR}/Monitor.hpp
	${AXLIBS_WINDIR}/Window.cpp
	${AXLIBS_WINDIR}/Window.hpp
)

set(TENSHI_DIR "Code/Tenshi")
set(TENSHI_CDIR "${TENSHI_DIR}/Compiler")
set(TENSHI_RDIR "${TENSHI_DIR}/Runtime")

add_executable(Tenshi
	${TENSHI_CDIR}/Resources/Compiler.rc
	${TENSHI_CDIR}/_PCH.cpp
	${TENSHI_CDIR}/_PCH.hpp
	${TENSHI_CDIR}/Binutils.cpp
	${TENSHI_CDIR}/Binutils.hpp
	${TENSHI_CDIR}/BuiltinType.cpp
	${TENSHI_CDIR}/BuiltinType.hpp
	${TENSHI_CDIR}/CodeGen.cpp
	${TENSHI_CDIR}/CodeGen.hpp
	${TENSHI_CDIR}/CodeGen_Cast.cpp
	${TENSHI_CDIR}/CodeGen_Expr.cpp
	${TENSHI_CDIR}/CodeGen_Labels.cpp
	${TENSHI_CDIR}/CodeGen_Main.cpp
	${TENSHI_CDIR}/CodeGen_Mods.cpp
	${TENSHI_CDIR}/CodeGen_Types.cpp
	${TENSHI_CDIR}/CodeGen_Writer.cpp
	${TENSHI_CDIR}/Environment.cpp
	${TENSHI_CDIR}/Environment.hpp
	${TENSHI_CDIR}/ExprParser.cpp
	${TENSHI_CDIR}/ExprParser.hpp
	${TENSHI_CDIR}/FunctionParser.cpp
	${TENSHI_CDIR}/FunctionParser.hpp
	${TENSHI_CDIR}/Lexer.cpp
	${TENSHI_CDIR}/Lexer.hpp
	${TENSHI_CDIR}/Main.cpp
	${TENSHI_CDIR}/Module.cpp
	${TENSHI_CDIR}/Module.hpp
	${TENSHI_CDIR}/Node.cpp
	${TENSHI_CDIR}/Node.hpp
	${TENSHI_CDIR}/Operator.hpp
	${TENSHI_CDIR}/Options.cpp
	${TENSHI_CDIR}/Options.hpp
	${TENSHI_CDIR}/Parser.cpp
	${TENSHI_CDIR}/Parser.hpp
	${TENSHI_CDIR}/Parser_Compile.cpp
	${TENSHI_CDIR}/ParserConfig.cpp
	${TENSHI_CDIR}/ParserConfig.hpp
	${TENSHI_CDIR}/Platform.cpp
	${TENSHI_CDIR}/Platform.hpp
	${TENSHI_CDIR}/Program.cpp
	${TENSHI_CDIR}/Program.hpp
	${TENSHI_CDIR}/Project.cpp
	${TENSHI_CDIR}/Project.hpp
	${TENSHI_CDIR}/Shell.cpp
	${TENSHI_CDIR}/Shell.hpp
	${TENSHI_CDIR}/StmtParser.cpp
	${TENSHI_CDIR}/StmtParser.hpp
	${TENSHI_CDIR}/Symbol.cpp
	${TENSHI_CDIR}/Symbol.hpp
	${TENSHI_CDIR}/Tester.cpp
	${TENSHI_CDIR}/Tester.hpp
	${TENSHI_CDIR}/TypeInformation.cpp
	${TENSHI_CDIR}/TypeInformation.hpp
	${TENSHI_CDIR}/UDTParser.cpp
	${TENSHI_CDIR}/UDTParser.hpp
)
target_link_libraries(Tenshi
	AxAllocation
	AxAsync
	AxCollections
	AxConsole
	AxCore
	AxInput
	AxMath
	AxParser
	AxPlatform
	AxSystem
	AxWindow
	${llvm_libs}
)

# Resulting library is used by the compiler at link time
add_library(TenshiRuntime
	${TENSHI_RDIR}/TenshiRuntime.c
	${TENSHI_RDIR}/TenshiRuntime.h
)

file(COPY "ThirdParty/GNU" DESTINATION ".")

# Modules

add_tenshi_api("API.FileSystem"
	../APICommon.hpp
	_PCH.cpp
	_PCH.hpp
	Directory.cpp
	Directory.hpp
	File.cpp
	File.hpp
	FileMgr.cpp
	FileMgr.hpp
	FindFiles.cpp
	Main.cpp
	Path.cpp
	Path.hpp
	StreamType.hpp
)
