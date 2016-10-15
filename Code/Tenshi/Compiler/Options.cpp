#include "_PCH.hpp"
#include "Options.hpp"

namespace Tenshi { namespace Compiler {

	MOptions &MOptions::GetInstance()
	{
		static MOptions instance;
		return instance;
	}

	MOptions::MOptions()
	: m_pOptions()
	, m_Inputs()
	{
		memset( &m_pShortOpts, 0, sizeof( m_pShortOpts ) );
	}
	MOptions::~MOptions()
	{
	}

	MOptions &MOptions::Register( IOption &Opt )
	{
		const char *const pszOptLongName = Opt.GetLongName();
		const char chOptShortName = Opt.GetShortName();
		const size_t uShortIndex = ( size_t )( unsigned char )chOptShortName;

		AX_ASSERT_NOT_NULL( pszOptLongName );

		AX_ASSERT_IS_NULL( Find( pszOptLongName ) );
		AX_ASSERT_IS_NULL( m_pShortOpts[ uShortIndex ] );

		if( uShortIndex > 0 ) {
			AX_ASSERT( uShortIndex < Ax::ArraySize( m_pShortOpts ) );

			m_pShortOpts[ uShortIndex ] = &Opt;
		}

		AX_EXPECT_MEMORY( m_pOptions.Append( &Opt ) );
		return *this;
	}

	const Ax::TArray<const char *> &MOptions::GetInputs() const
	{
		return m_Inputs;
	}
	const Ax::TArray<IOption *> &MOptions::GetOptions() const
	{
		return m_pOptions;
	}

	bool MOptions::Process( int argc, char **argv )
	{
		unsigned cSuccesses = 0;
		unsigned cFailures = 0;

		bool bOnlyInputs = false;

		for( int i = 1; i < argc; ++i ) {
			const char *const basearg = argv[ i ];
			const char *arg = basearg;

			if( *arg != '-' || bOnlyInputs ) {
				AX_EXPECT_MEMORY( m_Inputs.Append( arg ) );
				++cSuccesses;
				continue;
			}

			++arg;

			bool bIsShort = true;
			if( *arg == '-' ) {
				bIsShort = false;
				++arg;
				if( *arg == '\0' ) {
					bOnlyInputs = true;
					continue;
				}
			}

			bool bIsNot = false;
			if( arg[0]=='n' && arg[1]=='o' && arg[2]=='-' ) {
				bIsNot = true;
				arg += 3;
			}

			bIsShort &= !bIsNot;

			EOptionArg ArgType = EOptionArg::None;
			const char *optarg = nullptr;
			IOption *pOpt = nullptr;
			if( bIsShort ) {
				pOpt = m_pShortOpts[ *( const unsigned char * )arg ];
				if( pOpt != nullptr ) {
					ArgType = pOpt->GetArgumentType();
					if( ArgType == EOptionArg::None ) {
						pOpt = nullptr;
					} else if( arg[1] != '\0' ) {
						++arg;
						optarg = arg;
					}
				}
			}

			if( !pOpt ) {
				const char *const s = arg;
				const char *const p = strchr( arg, '=' );
				const char *const e = p != nullptr ? p : strchr( arg, '\0' );
				if( p != nullptr ) {
					optarg = p + 1;
				}

				const Ax::uintptr n = e - s;

				pOpt = Find( s, n );
				if( !pOpt ) {
					Ax::BasicErrorf( "Unrecognized option '%.*s'", s, n );
					++cFailures;
					continue;
				}

				ArgType = pOpt->GetArgumentType();
			}

			AX_ASSERT_NOT_NULL( pOpt );

			const bool bNeedsParm = pOpt->IsArgumentRequired();

			if( ( ArgType == EOptionArg::None || ArgType == EOptionArg::Boolean ) && optarg != nullptr ) {
				Ax::BasicWarnf( "Option '%s' does not take an argument, got '%s'", pOpt->GetLongName(), optarg );
			}
				
			if( ArgType != EOptionArg::Boolean && bIsNot ) {
				Ax::BasicWarnf( "Option '%s' is not a switch, but was prefixed with 'no-'", pOpt->GetLongName() );
			}

			if( ArgType != EOptionArg::None && !optarg && bNeedsParm ) {
				if( i + 1 == argc ) {
					Ax::BasicErrorf( "Option '%s' expected a parameter", pOpt->GetLongName() );
					++cFailures;
					continue;
				}

				optarg = argv[ ++i ];
			}

			bool bOptResult = false;

			switch( ArgType )
			{
			case EOptionArg::None:
			case EOptionArg::Boolean:
				bOptResult = pOpt->Call( !bIsNot );
				break;

			// FIXME: Handle 'File', 'Directory', and 'OutputFile' differently
			case EOptionArg::File:
			case EOptionArg::Directory:
			case EOptionArg::OutputFile:
			case EOptionArg::String:
				bOptResult = pOpt->Call( optarg );
				break;

			case EOptionArg::Integer:
			case EOptionArg::RangedInteger:
				{
					const int iValue = ( int )Ax::String::ToInteger( optarg );
					if( ArgType == EOptionArg::RangedInteger ) {
						const int iMin = pOpt->GetMinRange();
						const int iMax = pOpt->GetMaxRange();

						if( iValue < iMin || iValue > iMax ) {
							Ax::BasicErrorf( "Option '%s' expects integers in the range (%i,%i), got %i", pOpt->GetLongName(), iMin, iMax, iValue );
							bOptResult = false;
							break;
						}
					}

					bOptResult = pOpt->Call( iValue );
				}
				break;

			case EOptionArg::Enum:
				{
					const size_t cEnums = pOpt->NumEnumItems();
					const char *const *const ppszEnums = pOpt->GetEnumItems();

					AX_ASSERT( cEnums > 0 );
					AX_ASSERT_NOT_NULL( ppszEnums );

					const size_t cArgLen = optarg != nullptr ? strlen( optarg ) : 0;

					Ax::uintptr EnumValue = ~( Ax::uintptr )0;
					for( Ax::uintptr j = 0; j < cEnums; ++j ) {
						AX_ASSERT_NOT_NULL( ppszEnums[ j ] );

						const size_t cItemLen = strlen( ppszEnums[ j ] );
						AX_ASSERT( cItemLen > 0 );

						if( cArgLen != cItemLen ) {
							continue;
						}

						if( optarg != nullptr && strcmp( ppszEnums[ j ], optarg ) == 0 ) {
							EnumValue = j;
							break;
						}
					}

					if( EnumValue >= cEnums ) {
						Ax::BasicErrorf( "Option '%s' expected one of the following values, but got '%s'", pOpt->GetLongName(), optarg );
						for( Ax::uintptr j = 0; j < cEnums; ++j ) {
							Ax::BasicStatusf( "\t* \"%s\"", ppszEnums[ j ] );
						}
						bOptResult = false;
					} else {
						bOptResult = pOpt->Call( EnumValue );
					}
				}
				break;

			default:
				AX_ASSERT_MSG( false, "Unhandled option argument type (EOptionArg)" );
				break;
			}

			if( !bOptResult ) {
				Ax::BasicErrorf( "Option '%s' (passed by \"%s\") did not work", pOpt->GetLongName(), basearg );
				++cFailures;
			} else {
				++cSuccesses;
			}
		}

		if( !cSuccesses && !cFailures ) {
			IOption *const pHelpOpt = Find( "help", 4 );
			if( pHelpOpt != nullptr ) {
				pHelpOpt->Call( "" );
			}

			return true;
		}

		return cSuccesses > 0;
	}

	IOption *MOptions::Find( const char *name, size_t n ) const
	{
		AX_ASSERT_NOT_NULL( name );
		if( n == ~( size_t )0 ) {
			n = strlen( name );
		}

		for( IOption *pCmpOpt : m_pOptions ) {
			AX_ASSERT_NOT_NULL( pCmpOpt );

			const char *const optname = pCmpOpt->GetLongName();
			AX_ASSERT_NOT_NULL( optname );

			const Ax::uintptr optnamelen = strlen( optname );
			AX_ASSERT( optnamelen > 0 );

			if( optnamelen != n ) {
				continue;
			}

			if( strncmp( optname, name, n ) == 0 ) {
				return pCmpOpt;
			}
		}

		return nullptr;
	}

}}
