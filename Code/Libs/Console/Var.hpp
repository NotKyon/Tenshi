#pragma once

#include "../Allocation/Allocator.hpp"
#include "../Collections/Dictionary.hpp"
#include "../Collections/List.hpp"
#include "../Core/String.hpp"

namespace Ax { namespace Console {

#define AX_CVAR_FLAGS\
	AX_CVAR_FLAG( User, "was declared by user" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( Cheat, "is a cheat" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( Server, "belongs to server" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( Client, "belongs to client" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( Init, "can only be set on command-line" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( Const, "cannot be set by user" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( Archive, "save to config file" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( MinArgs, "has minimum arguments" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( MaxArgs, "has maximum arguments" ) AX_CVAR_FLAG_DELM\
	AX_CVAR_FLAG( Modified, "was modified" )

	namespace Detail
	{

#define AX_CVAR_FLAG( x, y ) kVar_##x
#define AX_CVAR_FLAG_DELM ,

		enum EVarBits
		{
			AX_CVAR_FLAGS
		};

#undef AX_CVAR_FLAG_DELM
#undef AX_CVAR_FLAG

	}

#define AX_CVAR_FLAG( x, y ) kVar_##x##_Bit = 1UL << Detail::kVar_##x
#define AX_CVAR_FLAG_DELM ,

	enum EVarFlags
	{
		AX_CVAR_FLAGS
	};

#undef AX_CVAR_FLAG_DELM
#undef AX_CVAR_FLAG

	class CVar;

	typedef int32( *FnVarProcessor )( CVar &Var, uintcpu cArgs, const char *const *ppszArgs );

	enum class EVarPhase
	{
		Initialization,
		Normal
	};

	class CVar
	{
	public:
		static CVar *				g_pVIsDevMode;
		static CVar *				g_pCSet;

		static void					InitSystem			();
		static CVar *				Find				( const char *pszName );

									CVar				();
		virtual						~CVar				();

		bool						Init				( const char *pszName, uint32 uSysFlags );
		bool						InitCommand			( const char *pszName, FnVarProcessor pfnCommand );
		void Fini();

		bool						SetString			( const char *pszNewValue );
		bool						SetInteger			( int32 NewValue );
		bool						SetFloat			( float fNewValue );

		const char *				GetString			() const;
		int							GetInteger			() const;
		float						GetFloat			() const;

		void						SetRange			( double fMin, double fMax );
		double						GetMinimumValue		() const;
		double						GetMaximumValue		() const;

		int							GetSystemFlags		() const;
		int							GetUserFlags		() const;

		bool						HasAnySystemFlags	( uint32 uFlags ) const;
		bool						HasAllSystemFlags	( uint32 uFlags ) const;

		bool						IsUser				() const;
		bool						IsCheat				() const;
		bool						IsServer			() const;
		bool						IsClient			() const;
		bool						IsInit				() const;
		bool						IsConst				() const;
		bool						IsArchive			() const;
		bool						IsUsingMinArgs		() const;
		bool						IsUsingMaxArgs		() const;
		bool						IsModified			() const;

		bool						CanModify			( EVarPhase Phase = EVarPhase::Normal ) const;

		void						SetUserFlags		( uint32 uFlags );
		void						AddUserFlags		( uint32 uFlags );
		void						RemoveUserFlags		( uint32 uFlags );
		bool						HasAnyUserFlags		( uint32 uFlags ) const;
		bool						HasAllUserFlags		( uint32 uFlags ) const;

		void						SetArgumentsLimit	( uintcpu cMin, uintcpu cMax );
		int							GetMinimumArguments	() const;
		int							GetMaximumArguments	() const;

		void						SetCommandProcessor	( FnVarProcessor pfnCommand );
		FnVarProcessor				GetCommandProcessor	() const;

		bool						InvokeCommand		( uintcpu cArgs, const char *const *ppszArgs );
		static bool					ExecuteCommands		( const char *pszSource, const char *pszFilename, int32 BaseLine );

	private:
		typedef TDictionary< CVar >::SEntry EntryType;

		struct State
		{
			TIntrusiveList< CVar >	List;
			TDictionary< CVar >		Dictionary;

			static State &			Get					();

									~State				();

		private:
									State				();
		};
		inline static State &		G					()
									{
										return State::Get();
									}

		String						m_Value;
		mutable int32				m_iValue;
		mutable float				m_fValue;

		double						m_fMinValue;
		double						m_fMaxValue;

		mutable uintcpu				m_uSysFlags;
		uintcpu						m_uUsrFlags;

		uintcpu						m_cMinParms;
		uintcpu						m_cMaxParms;
		FnVarProcessor				m_pfnProcessor;

		EntryType *					m_pEntry;
		TIntrusiveLink< CVar >		m_Link;

		void						CalculateNumberValuesNoRange() const;
		void						CalculateNumberValues() const;
	};

	CVar *GetDevModeVar();
	CVar *FindVar( const char *pszName );

	CVar *NewVar( const char *pszName, uint32 uSysFlags = 0, int memtag = AX_DEFAULT_MEMTAG );
	CVar *NewCommandVar( const char *pszName, FnVarProcessor pfnCommand, int memtag = AX_DEFAULT_MEMTAG );
	CVar *DeleteVar( CVar *pVar );

}}
