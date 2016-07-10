#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Core/Manager.hpp>

#include <Collections/Array.hpp>
#include <Collections/List.hpp>

#include "Symbol.hpp"

namespace Tenshi { namespace Compiler {

	enum class EModule
	{
		// Module is an internal system (defined by the compiler internally)
		Internal,
		// Module is a static library (linked directly against)
		StaticLibrary,
		// Module is a dynamic library (loaded at runtime)
		DynamicLibrary,
		// Module is part of the user's code (e.g., a framework)
		UserCode
	};

	struct SModuleFunc
	{
		Ax::String					Name;
		llvm::Function *			pFunc;
	};

	struct SModule
	{
		typedef Ax::TIntrusiveList<SModule>				IntrList;
		typedef Ax::TIntrusiveLink<SModule>				IntrLink;

		// Name of the module (e.g., "AxTech.Jin.Basic3D")
		Ax::String					Name;
		// Name of the object file (e.g., "win32/x86/plugins-core/AxTech.Jin.Basic3D.dll")
		Ax::String					Filename;
		// Name of the object file (for debugging)
		Ax::String					DebugFilename;
		// Type of the module (e.g., EModule::DynamicLibrary)
		EModule						Type;
		// Initialization function in the module (e.g., "Jin_Basic3D_Init")
		SModuleFunc					InitFunction;
		// Name of the finishing function in the module (e.g., "Jin_Basic3D_Fini")
		SModuleFunc					FiniFunction;
		// Name of the function called internally by safety core in loops (e.g., "Jin_Basic3D_SafeLoop")
		SModuleFunc					LoopFunction;
		// Name of the function called by SYNC automatically (e.g., "Jin_Basic3D_Step")
		SModuleFunc					StepFunction;
		// Name of the function called when the app is hidden (i.e., switched away from)
		SModuleFunc					HideFunction;
		// Name of the function called when the app is shown (i.e., switched to)
		SModuleFunc					ShowFunction;
		// Name of the function called when the app needs to save its internal state
		SModuleFunc					SaveFunction;
		// Name of the function called when the app needs to load its internal state
		SModuleFunc					LoadFunction;
		// List of all the modules this depends upon (e.g., ["AxTech.Jin.Renderer","AxTech.Jin.Entity"])
		Ax::TArray< Ax::String >	DependencyNames;
		// Pointers to all loaded modules found (each index corresponds to the name in DependencyNames)
		Ax::TArray< const SModule * > Dependencies;
		// All of the symbols defined by this module
		CScope						Definitions;

		IntrLink					SourceLink;
		IntrLink					ProjectLink;

		inline SModule()
		: Name()
		, Filename()
		, DebugFilename()
		, Type( EModule::Internal )
		, InitFunction()
		, FiniFunction()
		, LoopFunction()
		, StepFunction()
		, HideFunction()
		, ShowFunction()
		, SaveFunction()
		, LoadFunction()
		, DependencyNames()
		, Dependencies()
		, Definitions()
		, SourceLink( this )
		, ProjectLink( this )
		{
		}
	};

	class MModules
	{
	public:
		static MModules &GetInstance();

		SModule *LoadCoreInternal();
		void LoadCorePlugins();

		SModule *LoadFromText( const Ax::String &Filename, const Ax::String &Text );
		SModule *LoadFromFile( const Ax::String &Filename );
		void LoadDirectory( const Ax::String &DirectoryName );

		const Ax::TList< SModule > &List() const;
		Ax::TList< SModule > &List();

	private:
		MModules();
		~MModules();

		Ax::TList< SModule >		m_Mods;
		int							m_iCurrentModId;

		SModule *					m_pCoreMod;
		bool						m_bLoadedCoreMods;

		AX_DELETE_COPYFUNCS(MModules);
	};
	static Ax::TManager< MModules >	Mods;

}}
