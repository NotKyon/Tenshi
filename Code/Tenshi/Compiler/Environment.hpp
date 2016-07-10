#pragma once

#include <Core/Manager.hpp>
#include <Collections/List.hpp>

#include "Symbol.hpp"
#include "Module.hpp"
#include "Platform.hpp"
#include "TypeInformation.hpp"
#include "Node.hpp" // For STypeRef (need to store in CEnvironment)

namespace Tenshi { namespace Compiler {

	typedef Ax::TList< SModule >	ModuleList;
	typedef ModuleList::Iterator	ModuleIter;

	// Environment scope (holds plug-ins)
	class CEnvironment
	{
	public:
		static CEnvironment &GetInstance();

		void SetBuildInfo( const SBuildInfo &BuildInfo );
		const SBuildInfo &BuildInfo() const;

		SymbolDictionary &Dictionary();
		const ModuleList &Modules() const;

		EBuiltinType GetIntPtrType() const;
		EBuiltinType GetUIntPtrType() const;
		Ax::uint32 GetPointerSizeInBytes() const;
		inline Ax::uint32 GetPointerBits() const
		{
			return GetPointerSizeInBytes()*8;
		}

		STypeRef &GetIntPtr() const;
		STypeRef &GetUIntPtr() const;

		// Searches the given directory path (no sub-paths) for module command files
		bool LoadModulesInPath( const Ax::String &InPath );
		// Takes a .commands file and fills in module information
		bool LoadModule( const Ax::String &InModPath );

		// Allocate a module directly
		SModule &AllocModule();

		inline bool IsDebugEnabled() const
		{
			return m_BuildInfo.Debugging != EDebugMode::NoSymbols;
		}

	private:
		SymbolDictionary			m_Dictionary;
		Ax::TList< SModule >		m_Modules;
		SBuildInfo					m_BuildInfo;
		STypeRef					m_IntPtrRTy;
		STypeRef					m_UIntPtrRTy;

		CEnvironment();
		~CEnvironment();
	};
	static Ax::TManager< CEnvironment > g_Env;

	//========================================================================//

	inline const SBuildInfo &CEnvironment::BuildInfo() const
	{
		return m_BuildInfo;
	}

	inline SymbolDictionary &CEnvironment::Dictionary()
	{
		return m_Dictionary;
	}
	inline const ModuleList &CEnvironment::Modules() const
	{
		return m_Modules;
	}

	inline Ax::uint32 CEnvironment::GetPointerSizeInBytes() const
	{
		return m_BuildInfo.Platform.GetPointerBytes();
	}

}}
