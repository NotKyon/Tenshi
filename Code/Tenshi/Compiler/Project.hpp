#pragma once

#include <Collections/List.hpp>
#include <Core/String.hpp>
#include <Core/Manager.hpp>

#include "Platform.hpp"
#include "Module.hpp"

namespace Tenshi { namespace Compiler {

	class MCodeGen;

	// Linking target type (executable, dynamic library, static library)
	enum class ELinkTarget
	{
		// No linking will take place
		None,
		// The output will be a native executable (Windows: .exe)
		Executable,
		// The output will be a dynamic library (Windows: .dll; Mac: .dylib; Linux: .so)
		DynamicLibrary,
		// The output will be a static library (.a)
		StaticLibrary
	};
	// Environment the target is to run in
	enum class ETargetEnv
	{
		// Text mode / terminal (e.g., server or pipeline tool)
		Terminal,
		// GUI mode (no default console box in Windows)
		Windowed,
		// Platform framework (e.g., phone or store app)
		Platform
	};
	// Link-Time Optimization configuration
	enum class ELTOConfig
	{
		Disabled,
		Enabled
	};
	
	// Level of debugging support in compilation
	enum class EDebugConfig
	{
		Disabled,
		Enabled
	};
	// Level of optimization support in compilation
	enum class EOptimizeConfig
	{
		Disabled,
		PerFunction,
		PerModule
	};

	// Settings for any given compilation
	struct SCompileSettings
	{
		EDebugConfig				Debug;
		EOptimizeConfig				Optimize;

		inline bool DebugEnabled() const
		{
			return Debug != EDebugConfig::Disabled;
		}
		inline bool OptimizationEnabled() const
		{
			return Optimize != EOptimizeConfig::Disabled;
		}
	};

	// A single compilation unit, with all relevant settings
	struct SCompilation
	{
		typedef Ax::TList< SCompilation >				List;
		typedef Ax::TList< SCompilation >::Iterator		Iter;

		// Settings for the source file
		SCompileSettings			Settings;

		// Input source filename
		Ax::String					SourceFilename;

		// Optional object filename (outputs to the object file format)
		Ax::String					ObjectFilename;
		// Optional bitcode filename (outputs to LLVM bitcode file; .bc)
		Ax::String					LLVMBCFilename;
		// Optional IR listing filename (outputs text representation of the module's LLVM IR; .ll)
		Ax::String					IRListFilename;
		// Optional assembly listing filename (outputs to target machine assembly representation; .s)
		Ax::String					ASListFilename;

		// Modules referenced by this compilation (only valid while this is active)
		SModule::IntrList			Modules;

		inline SCompilation()
		: Settings()
		, SourceFilename()
		, ObjectFilename()
		, LLVMBCFilename()
		, IRListFilename()
		, ASListFilename()
		, Modules()
		{
		}
		inline ~SCompilation()
		{
		}

		bool Build();
	};

	// Token used when parsing the project file
	struct SProjToken
	{
		const char *				s;
		const char *				e;

		inline bool IsQuote() const
		{
			return s != nullptr && *s != '\"';
		}

		inline bool Cmp( const char *pszCmp ) const
		{
			AX_ASSERT_NOT_NULL( s );
			AX_ASSERT_NOT_NULL( pszCmp );

			const Ax::uintptr myLen = e - s;
			const Ax::uintptr n = strlen( pszCmp );

			if( myLen != n ) {
				return false;
			}

			return strncmp( s, pszCmp, n ) == 0;
		}

		inline bool operator==( const char *pszCmp ) const
		{
			return Cmp( pszCmp );
		}
		inline bool operator!=( const char *pszCmp ) const
		{
			return !Cmp( pszCmp );
		}

		bool Unquote( Ax::String &Dst, bool bAppend = false ) const;
	};
	// Diagnostic state for the project file parser
	struct SProjDiagState
	{
		const char *				pszFilename;
		Ax::uint32					uLine;

		const char *				pszLineStart;

		inline Ax::uint32 Column( const char *p ) const
		{
			return 1 + ( Ax::uint32 )( p - pszLineStart );
		}

		inline bool Error( const char *p, const char *pszError )
		{
			AX_ASSERT_NOT_NULL( p );

			Ax::g_ErrorLog( pszFilename, uLine, Column( p ) ) += pszError;
			return false;
		}
		inline bool Error( const char *p, const Ax::String &ErrorStr )
		{
			AX_ASSERT_NOT_NULL( p );

			Ax::g_ErrorLog( pszFilename, uLine, Column( p ) ) += ErrorStr;
			return false;
		}
		
		inline bool Error( const SProjToken &Tok, const char *pszError )
		{
			Ax::g_ErrorLog( pszFilename, uLine, Column( Tok.s ) ) += pszError;
			return false;
		}
		inline bool Error( const SProjToken &Tok, const Ax::String &ErrorStr )
		{
			Ax::g_ErrorLog( pszFilename, uLine, Column( Tok.s ) ) += ErrorStr;
			return false;
		}
	};

	// Whether variable arguments can be used
	enum class EVarArgs
	{
		No,
		Yes
	};

	// Encapsulation of an individual target
	class CProject
	{
	public:
		typedef Ax::TList< CProject >					List;
		typedef Ax::TList< CProject >::Iterator			Iter;

		typedef Ax::TList< CProject * >					PtrList;
		typedef Ax::TList< CProject * >::Iterator		PtrIter;

		// Load the project's settings from a file
		bool LoadFromFile( const char *pszFilename );
		// Apply a single line (as though it were from a project file)
		bool ApplyLine( const char *pszFilename, Ax::uint32 uLine, const char *pszLineStart, const char *pszLineEnd = nullptr );

		// Build the project (returns false if it failed)
		bool Build();

		// Add the module to the current compilation/project
		void TouchModule( SModule &Mod );

	private:
		friend class MProjects;
		friend class List;

		CProject();
		~CProject();

	private:
		friend class MCodeGen;

		// Name of this project (this is mostly meaningless)
		Ax::String					m_Name;
		// Path to the source files
		Ax::String					m_SourceDir;
		// Path to the temporary object files
		Ax::String					m_ObjectDir;
		// Full path to the target file
		Ax::String					m_TargetPath;
		// Whether a prefix should be applied when setting the target path
		bool						m_bTargetPrefix:1;
		// Whether a suffix should be applied when setting the target path
		bool						m_bTargetSuffix:1;
		// Whether an assembly file listing should be produced
		bool						m_bASMList:1;
		// Whether a LLVM IR file listing should be produced
		bool						m_bIRList:1;

		// Target link type (e.g., executable)
		ELinkTarget					m_TargetType;
		// Target environment (e.g., gui)
		ETargetEnv					m_TargetEnv;
		// Link-time optimization (LTO) setting for this project
		ELTOConfig					m_LTO;

		// Build information for this project
		SBuildInfo					m_BuildInfo;

		// Current settings -- applied as the default settings when new translation units are added
		SCompileSettings			m_Settings;

		// Compilation units
		SCompilation::List			m_Compilations;
		// Current compilation
		SCompilation *				m_pCurrentCompilation;

		// Modules used by this project (only valid while project is being built)
		SModule::IntrList			m_Modules;

		AX_DELETE_COPYFUNCS(CProject);
	};

	// Project manager
	class MProjects
	{
	public:
		static MProjects &GetInstance();

		bool AddProjectDirectory( const char *pszProjDir );

		CProject &Current();

		CProject *Add();
		CProject *Load( const char *pszProjFile );

		CProject *FindExisting( const char *pszName, const char *pszNameEnd = nullptr ) const;
		inline CProject *FindExisting( const Ax::String &Name ) const
		{
			return FindExisting( Name.CString(), Name.CString() + Name.Len() );
		}

		// Remove the given project (freeing its memory) -- returns nullptr
		CProject *Remove( CProject *pProj );
		void Clear();

		bool Build();

	private:
		Ax::TList< Ax::String >		m_ProjectDirs;
		CProject::List				m_Projects;
		CProject::Iter				m_CurrProj;

		MProjects();
		~MProjects();

		AX_DELETE_COPYFUNCS(MProjects);
	};
	static Ax::TManager< MProjects >	Projects;

	// Push a token to a static array of tokens
	template< Ax::uintptr kNumTokens >
	inline bool PushToken( SProjToken( &Tokens )[ kNumTokens ], Ax::uintptr &uIndex, const char *s, const char *e )
	{
		AX_ASSERT_NOT_NULL( s );
		AX_ASSERT_NOT_NULL( e );
		AX_ASSERT_MSG( Ax::uintptr( s ) < Ax::uintptr( e ), "Invalid token string" );

		if( uIndex >= kNumTokens ) {
			return false;
		}

		Tokens[ uIndex ].s = s;
		Tokens[ uIndex ].e = e;
		++uIndex;
		
		return true;
	}
	// Check for a parameter in an array of parameters
	template< Ax::uintptr kNumTokens >
	inline bool HasParm( SProjToken( &Tokens )[ kNumTokens ], Ax::uintptr cTokens, SProjDiagState &Diag, const SProjToken &CmdTok, Ax::uintptr cParms = 1, EVarArgs VarArgs = EVarArgs::No, Ax::uintptr cMaxParms = ~Ax::uintptr( 0 ) )
	{
		AX_ASSERT( cTokens <= kNumTokens );
		AX_ASSERT( &CmdTok >= &Tokens[0] && &CmdTok < &Tokens[kNumTokens] );

		const Ax::uintptr uCmd = &CmdTok - &Tokens[0];
		AX_ASSERT( uCmd < kNumTokens );

		const Ax::uintptr cArgs = cTokens - uCmd - 1;

		if( ( cArgs == cParms || ( VarArgs == EVarArgs::Yes && cArgs > cParms ) ) && cArgs <= cMaxParms ) {
			return true;
		}

		if( cArgs < cParms ) {
			Diag.Error( CmdTok, "Too few parameters" );
			return false;
		}

		Diag.Error( CmdTok, "Too many parameters" );
		return false;
	}

}}
