#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>

namespace Tenshi { namespace Compiler {

	enum class ESubsystem
	{
		// App meant to run in the background, without user intervention (e.g., a daemon)
		Background,
		// App is for console/terminal (text mode)
		Text,
		// App is meant for PC with GUI
		Window,
		// App compatible with all app store requirements (requires EExecutable::Normal)
		Store
	};

	enum EPointerSize
	{
		// Pointers are 32-bit
		kPointer32,
		// Pointers are 64-bit
		kPointer64
	};
	enum class EEndianMode
	{
		// Platform Endianness might be configurable
		Unknown,
		// Platform is always Little Endian (0x1234 is stored 0x34 0x12 in memory)
		Little,
		// Platform is always Big Endian (0x1234 is stored 0x12 0x34 in memory)
		Big
	};

	enum class EBuildType
	{
		// This is a development build
		Development,
		// This is a build for testers
		Testing,
		// This is the finished build ready for release
		Release
	};

	enum class EDebugMode
	{
		// Do not generate debug symbols
		NoSymbols,
		// Generate debug symbols to an external file
		ExternalSymbols,
		// Generate debug symbols into the executable / objects
		InternalSymbols
	};
	enum class EProfileMode
	{
		// Do not enable instrumentation
		NoProfiling,
		// Enable instrumentation on functions
		FunctionProfiling
	};
	enum class ESafetyCode
	{
		// Do not generate calls to SYNC in any loop except DO/LOOP
		Off,
		// Generate calls to SYNC in all loops except FOR/NEXT
		On
	};
	enum class EExecutable
	{
		// Do not do anything special for the executable (must be set for ESubsystem::Store)
		Normal,
		// Make the executable stand-alone
		StandAlone,
		// Put all assets into a media database, but leave runtime DLLs out
		MediaDatabase,
		// Package all DLLs and libraries separately from the executable
		PackDependencies
	};

	struct SPlatform
	{
		Ax::String					OSName;
		Ax::String					ExeExt;
		Ax::String					DLLExt;
		ESubsystem					Subsystem;
		EPointerSize				PointerSize;
		EEndianMode					Endianness;
		Ax::uint16					TypeInstanceAlignment;
		Ax::uint16					TypeMemberAlignment;

		inline Ax::uint32 GetPointerBytes() const
		{
			switch( PointerSize )
			{
			case kPointer32:		return 4;
			case kPointer64:		return 8;
			}

			return 0;
		}
	};

	struct SBuildInfo
	{
		SPlatform					Platform;
		EBuildType					Type;
		EDebugMode					Debugging;
		EProfileMode				Profiling;
		ESafetyCode					SafetyCode;
		EExecutable					Executable;
	};

}}