#include "_PCH.hpp"
#include "Environment.hpp"
#include "ParserConfig.hpp"
#include "TypeInformation.hpp"
#include "Node.hpp"

namespace Tenshi { namespace Compiler {

	CEnvironment &CEnvironment::GetInstance()
	{
		static CEnvironment instance;
		return instance;
	}

	CEnvironment::CEnvironment()
	: m_Dictionary()
	, m_Modules()
	, m_BuildInfo()
	, m_IntPtrRTy()
	, m_UIntPtrRTy()
	{
		AX_EXPECT_MSG( m_Dictionary.Init( TENSHI_PARSER_DICTIONARY_ALLOWED, SourceCasing ), "Out of memory" );

		AX_EXPECT_MSG( m_BuildInfo.Platform.OSName.Assign( "Windows" ), "Out of memory" );
		AX_EXPECT_MSG( m_BuildInfo.Platform.ExeExt.Assign( ".exe" ), "Out of memory" );
		AX_EXPECT_MSG( m_BuildInfo.Platform.DLLExt.Assign( ".dll" ), "Out of memory" );
		m_BuildInfo.Platform.Subsystem = ESubsystem::Text;
		m_BuildInfo.Platform.PointerSize = EPointerSize::kPointer64;
		m_BuildInfo.Platform.Endianness = EEndianMode::Little;
		m_BuildInfo.Platform.TypeInstanceAlignment = 16;
		m_BuildInfo.Platform.TypeMemberAlignment = 4;

		m_BuildInfo.Type = EBuildType::Development;
		m_BuildInfo.Debugging = EDebugMode::InternalSymbols;
		m_BuildInfo.Profiling = EProfileMode::NoProfiling;
		m_BuildInfo.SafetyCode = ESafetyCode::On;
		m_BuildInfo.Executable = EExecutable::Normal;
	}
	CEnvironment::~CEnvironment()
	{
	}

	void CEnvironment::SetBuildInfo( const SBuildInfo &BuildInfo )
	{
		m_BuildInfo = BuildInfo;

		m_IntPtrRTy.Access = EAccess::ReadOnly;
		m_IntPtrRTy.bCanReorderMemAccesses = true;
		m_IntPtrRTy.BuiltinType = GetIntPtrType();
		m_IntPtrRTy.cBytes = GetPointerSizeInBytes();
		m_IntPtrRTy.pCustomType = nullptr;
		m_IntPtrRTy.pRef = nullptr;
		m_IntPtrRTy.Translated.bDidTranslate = false;
		m_IntPtrRTy.Translated.pType = nullptr;

		m_UIntPtrRTy.Access = EAccess::ReadOnly;
		m_UIntPtrRTy.bCanReorderMemAccesses = true;
		m_UIntPtrRTy.BuiltinType = GetUIntPtrType();
		m_UIntPtrRTy.cBytes = GetPointerSizeInBytes();
		m_UIntPtrRTy.pCustomType = nullptr;
		m_UIntPtrRTy.pRef = nullptr;
		m_UIntPtrRTy.Translated.bDidTranslate = false;
		m_UIntPtrRTy.Translated.pType = nullptr;
	}

	EBuiltinType CEnvironment::GetIntPtrType() const
	{
		switch( m_BuildInfo.Platform.PointerSize )
		{
		case kPointer32:			return EBuiltinType::Int32;
		case kPointer64:			return EBuiltinType::Int64;
		}

		AX_ASSERT_MSG( false, "Unreachable" );
		return EBuiltinType::Int64;
	}
	EBuiltinType CEnvironment::GetUIntPtrType() const
	{
		switch( m_BuildInfo.Platform.PointerSize )
		{
		case kPointer32:			return EBuiltinType::UInt32;
		case kPointer64:			return EBuiltinType::UInt64;
		}

		AX_ASSERT_MSG( false, "Unreachable" );
		return EBuiltinType::UInt64;
	}

	STypeRef &CEnvironment::GetIntPtr() const
	{
		return const_cast< STypeRef & >( m_IntPtrRTy );
	}
	STypeRef &CEnvironment::GetUIntPtr() const
	{
		return const_cast< STypeRef & >( m_UIntPtrRTy );
	}

	bool CEnvironment::LoadModulesInPath( const Ax::String &InPath )
	{
		Ax::System::CFileList Files;
		if( !Ax::System::EnumFileTree( Files, InPath ) ) {
			return false;
		}

		Files.FilterExtension( ".commands" );

		bool bDidAllFail = !Files.IsEmpty();
		for( Ax::uintptr i = 0; i < Files.Num(); ++i ) {
			bDidAllFail &= !LoadModule( Files.GetFile( i ) );
		}

		return !bDidAllFail;
	}
	bool CEnvironment::LoadModule( const Ax::String &InModPath )
	{
		Ax::String FileText;

		if( !Ax::System::ReadFile( FileText, InModPath ) ) {
			return false;
		}

		Ax::Debugf( InModPath, ".command file parsing not yet implemented (dummy success)" );

		return true;
	}

	SModule &CEnvironment::AllocModule()
	{
		ModuleIter Mod = m_Modules.AddTail();
		AX_EXPECT_MSG( Mod != m_Modules.end(), "Out of memory" );

		return *Mod.Get();
	}

}}
