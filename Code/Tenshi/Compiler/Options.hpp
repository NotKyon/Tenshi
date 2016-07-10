#pragma once

#include <Core/Manager.hpp>
#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Collections/List.hpp>
#include <Collections/Array.hpp>

namespace Tenshi { namespace Compiler {

	enum class EOptionArg
	{
		None,
		Boolean,
		File,
		Directory,
		OutputFile,
		String,
		Integer,
		RangedInteger,
		Enum
	};
	union UOptionArg
	{
		bool						bValue;
		const char *				pszValue;
		int							iValue;
		Ax::uintptr					EnumValue;
	};

	class IOption
	{
	public:
		IOption() {}
		virtual ~IOption() {}

		virtual char GetShortName() const { return '\0'; }
		virtual const char *GetLongName() const { return nullptr; }
		virtual const char *GetBriefHelp() const { return nullptr; }
		virtual const char *GetDetailedHelp() const { return nullptr; }
		virtual EOptionArg GetArgumentType() const { return EOptionArg::None; }
		virtual bool IsArgumentRequired() const { return true; }
		virtual bool ShouldShowInHelp() const { return false; }

		virtual size_t NumEnumItems() const { return 0; }
		virtual const char *const *GetEnumItems() const { return nullptr; }

		virtual int GetMinRange() const { return 0; }
		virtual int GetMaxRange() const { return 0; }

		virtual bool OnCall( UOptionArg Arg ) { return false; }

		bool Call( bool bValue ) { return Call_( bValue ); }
		bool Call( const char *pszValue ) { return Call_( pszValue ); }
		bool Call( int iValue ) { return Call_( iValue ); }
		bool Call( Ax::uintptr EnumValue ) { return Call_( EnumValue ); }

	private:
		template< typename tElement >
		inline bool Call_( tElement Element )
		{
			static_assert( sizeof( tElement ) <= sizeof( Ax::uintptr ), "Invalid typename used for element" );

			UOptionArg Arg;
			Arg.EnumValue = ( Ax::uintptr )Element;

			return OnCall( Arg );
		}
	};

	class MOptions
	{
	public:
		static MOptions &GetInstance();

		MOptions &Register( IOption &Opt );

		const Ax::TArray<IOption *> &GetOptions() const;
		const Ax::TArray<const char *> &GetInputs() const;
		bool Process( int argc, char **argv );

		IOption *Find( const char *name, size_t n = ~( size_t )0 ) const;

	private:
		MOptions();
		~MOptions();

		Ax::TArray<IOption *>		m_pOptions;
		IOption *					m_pShortOpts[ 256 ];

		Ax::TArray<const char *>	m_Inputs;
	};
	static Ax::TManager<MOptions>	Opts;

}}