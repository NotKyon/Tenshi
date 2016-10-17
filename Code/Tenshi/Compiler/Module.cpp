#include "_PCH.hpp"
#include "Module.hpp"
#include "Environment.hpp"
#include "ParserConfig.hpp"

#include <System/FileSystem.hpp>

namespace Tenshi { namespace Compiler {

	using namespace Ax;
	
	MModules &MModules::GetInstance()
	{
		static MModules instance;
		return instance;
	}

	MModules::MModules()
	: m_Mods()
	, m_iCurrentModId( 0 )
	, m_pCoreMod( nullptr )
	, m_bLoadedCoreMods( false )
	{
	}
	MModules::~MModules()
	{
	}

	SModule *MModules::LoadCoreInternal()
	{
		if( m_pCoreMod != nullptr ) {
			return m_pCoreMod;
		}

#define NL "\n"
		m_pCoreMod = LoadFromText
		(
			// Filename
			"$core",

			// Text
			".name $core"											NL
			""														NL
			"__AUTOPRINT%LS%teAutoPrint"							NL
			"__SAFELOOP%0%teSafeLoop"								NL
			"__STRINGCONCAT[%GSS%teStrCat"							NL
			"__STRINGFINDRM[%GSS%teStrFindRm"						NL
			"__STRINGREPEAT[%GSL%teStrRepeat"						NL
			"__STRINGCATPATH[%GSS%teStrCatPath"						NL
			""														NL
			"STR$[%GS%teStrDup"										NL
			"STR$[%GC%teCastInt8ToStr"								NL
			"STR$[%GN%teCastInt16ToStr"								NL
			"STR$[%GL%teCastInt32ToStr"								NL
			"STR$[%GR%teCastInt64ToStr"								NL
			"STR$[%GY%teCastUInt8ToStr"								NL
			"STR$[%GW%teCastUInt16ToStr"							NL
			"STR$[%GD%teCastUInt32ToStr"							NL
			"STR$[%GQ%teCastUInt64ToStr"							NL
			""														NL
			"ASC[%LS%teStr_Asc"										NL
			"CHR$[%GD%teStr_Chr"									NL
			""														NL
			"BIN$[%GD%teStr_Bin"									NL
			"HEX$[%GD%teStr_Hex"									NL
			"OCT$[%GD%teStr_Oct"									NL
			""														NL
			"LOWER$[%GS%teStr_Lower"								NL
			"UPPER$[%GS%teStr_Upper"								NL
			""														NL
			"LEN[%UPS%teStr_Len"									NL
			"STRCMP[%LSS%teStr_SortCmp"								NL
			"STRCMPCASE[%LSS%teStr_SortCmpCase"						NL
			""														NL
			"LEFT$[%GSIP%teStr_Left"								NL
			"MID$[%GSIP%teStr_Mid"									NL
			"MID$[%GSIPUP%teStr_MidLen"								NL
			"RIGHT$[%GSIP%teStr_Right"								NL
			"SKIP$[%GSUP%teStr_Skip"								NL
			"DROP$[%GSUP%teStr_Drop"								NL
			"HAS PREFIX$[%BSS%teStr_HasPrefix"						NL
			"HAS SUFFIX$[%BSS%teStr_HasSuffix"						NL
			"CONTAINS$[%BSS%teStr_Contains"							NL
			""														NL
			"MAKE MEMBLOCK[%LUP%teAllocMemblock"					NL
			"MAKE MEMBLOCK%LUP%teMakeMemblock"						NL
			"DELETE MEMBLOCK[%LL%teDeleteMemblock"					NL
			"MEMBLOCK EXIST[%BL%teMemblockExist"					NL
			"GET MEMBLOCK PTR[%UPL%teGetMemblockPtr"				NL
			"GET MEMBLOCK SIZE[%UPL%teGetMemblockSize"				NL
			"MEMBLOCK BYTE[%YLUP%teMemblockByte"					NL
			"MEMBLOCK WORD[%WLUP%teMemblockWord"					NL
			"MEMBLOCK DWORD[%DLUP%teMemblockDword"					NL
			"MEMBLOCK QWORD[%QLUP%teMemblockQword"					NL
			"MEMBLOCK FLOAT[%FLUP%teMemblockFloat"					NL
			"MEMBLOCK FLOAT64[%OLUP%teMemblockFloat64"				NL
			"WRITE MEMBLOCK BYTE%LUPY%teWriteMemblockByte"			NL
			"WRITE MEMBLOCK WORD%LUPW%teWriteMemblockWord"			NL
			"WRITE MEMBLOCK DWORD%LUPD%teWriteMemblockDword"		NL
			"WRITE MEMBLOCK QWORD%LUPQ%teWriteMemblockQword"		NL
			"WRITE MEMBLOCK FLOAT%LUPF%teWriteMemblockFloat"		NL
			"WRITE MEMBLOCK FLOAT64%LUPO%teWriteMemblockFloat64"	NL
			"COPY MEMBLOCK%LLUPUPUP%teCopyMemblock"					NL
			""														NL
		);
#undef NL

		AX_EXPECT_MEMORY( m_pCoreMod );
		m_pCoreMod->Type = EModule::Internal;
		return m_pCoreMod;
	}
	void MModules::LoadCorePlugins()
	{
		Ax::String PluginsCoreDir;
		char szAppDir[ PATH_MAX ];

		if( m_bLoadedCoreMods ) {
			return;
		}

		AX_EXPECT_NOT_NULL( Ax::System::GetAppDir( szAppDir ) );

		AX_EXPECT_MEMORY( PluginsCoreDir.Assign( szAppDir ) );
		AX_EXPECT_MEMORY( PluginsCoreDir.AppendPath( "plugins-core" ) );

		AX_DEBUG_LOG += "CoreMods: \"" + PluginsCoreDir + "\"";

		LoadDirectory( PluginsCoreDir );

		m_bLoadedCoreMods = true;
	}

	static bool IsOldstyleLine( const Ax::String &Line )
	{
		const intptr iPrcnt = Line.Find( '%' );
		const intptr iParen = Line.Find( '(' );

		return iPrcnt != -1 && ( iParen == -1 || iPrcnt < iParen );
	}

	SModule *MModules::LoadFromText( const Ax::String &Filename, const Ax::String &Text )
	{
		Ax::TArray< Ax::String > Lines;
		Ax::TArray< Ax::String > Parts;
		Ax::uint32 uLineNum = 0;

		Ax::TList< SModule >::Iterator ModIter = m_Mods.AddTail();
		AX_EXPECT_MEMORY( ModIter != m_Mods.end() );

		SModule &Mod = *ModIter;

		char szAbsPath[ PATH_MAX + 1 ];
		if( !GetAbsolutePath( szAbsPath, Filename ) ) {
			StrCpy( szAbsPath, Filename );
		}

		const Ax::String ModDir = Ax::String( szAbsPath ).ExtractDirectory();
		AX_EXPECT_MEMORY( ModDir.IsEmpty() == false );

		Ax::String ObjectFilename;
		Ax::String ObjectDbgFilename;

		Mod.Name = Filename.ExtractBasename();

		g_VerboseLog( Filename ) += "Setting module name: \"" + Mod.Name + "\"";

		Mod.Definitions.SetDictionary( g_Env->Dictionary(), "#M" + Ax::String::Formatted( "%.3X", m_iCurrentModId++ ) + "$" );

		Lines = Text.Split( "\n" );

		bool bIgnoring = false;

		for( Ax::String &Line : Lines ) {
			++uLineNum;
			Line.Trim();

			if( Line.StartsWith( "//" ) || Line.StartsWith( "`" ) || Line.IsEmpty() ) {
				continue;
			}

			// Manage target conditionals
			if( Line.StartsWith( "?" ) ) {
				if( Line == "?" ) {
					bIgnoring = false;
				} else if( Line == "?Windows" ) {
					bIgnoring = g_Env->BuildInfo().Platform.OSName != "Windows";
				} else if( Line == "?Linux" ) {
					bIgnoring = g_Env->BuildInfo().Platform.OSName != "Linux";
				} else if( Line == "?MacOSX" ) {
					bIgnoring = g_Env->BuildInfo().Platform.OSName != "MacOSX";
				} else if( Line == "?Debug" ) {
					bIgnoring = g_Env->BuildInfo().Debugging == EDebugMode::NoSymbols;
				} else if( Line == "?Release" ) {
					bIgnoring = g_Env->BuildInfo().Debugging != EDebugMode::NoSymbols;
				} else if( Line == "?x64" ) {
					// No other processor is supported right now...
					bIgnoring = false;
				} else {
					bIgnoring = true;
				}

				continue;
			}

			if( bIgnoring ) {
				continue;
			}

			//g_VerboseLog( Filename, uLineNum ) += "Have command line \"" + Line + "\"";

			if( Line.StartsWith( "." ) ) {

				Line.Replace( "\t", " " );
				if( !Line.Split( Parts, " ", Ax::EKeepEmpty::No ) ) {
					g_ErrorLog( Filename, uLineNum ) += "Failed to split line (Out of memory)";
					continue;
				}

				if( Parts.IsEmpty() ) {
					g_ErrorLog( Filename, uLineNum ) += "Ran out of memory while parsing line \"" + Line + "\"";
					continue;
				}

				Ax::uint32 cExpectedArgs = 0;
				bool bAssignResult = true;
				bool bAlreadyAssigned = false;

				if( Parts[0] == ".fn-init" ) {
					if( Parts.Num() == 2 ) {
						if( Mod.InitFunction.Name.IsEmpty() ) {
							bAssignResult = Mod.InitFunction.Name.Assign( Parts[1] );
						} else {
							bAlreadyAssigned = true;
						}
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".fn-fini" ) {
					if( Parts.Num() == 2 ) {
						if( Mod.FiniFunction.Name.IsEmpty() ) {
							bAssignResult = Mod.FiniFunction.Name.Assign( Parts[1] );
						} else {
							bAlreadyAssigned = true;
						}
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".fn-loop" ) {
					if( Parts.Num() == 2 ) {
						if( Mod.LoopFunction.Name.IsEmpty() ) {
							bAssignResult = Mod.LoopFunction.Name.Assign( Parts[1] );
						} else {
							bAlreadyAssigned = true;
						}
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".fn-step" ) {
					if( Parts.Num() == 2 ) {
						if( Mod.StepFunction.Name.IsEmpty() ) {
							bAssignResult = Mod.StepFunction.Name.Assign( Parts[1] );
						} else {
							bAlreadyAssigned = true;
						}
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".fn-hide" ) {
					if( Parts.Num() == 2 ) {
						if( Mod.HideFunction.Name.IsEmpty() ) {
							bAssignResult = Mod.HideFunction.Name.Assign( Parts[1] );
						} else {
							bAlreadyAssigned = true;
						}
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".fn-show" ) {
					if( Parts.Num() == 2 ) {
						if( Mod.ShowFunction.Name.IsEmpty() ) {
							bAssignResult = Mod.ShowFunction.Name.Assign( Parts[1] );
						} else {
							bAlreadyAssigned = true;
						}
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".fn-save" ) {
					if( Parts.Num() == 2 ) {
						if( Mod.SaveFunction.Name.IsEmpty() ) {
							bAssignResult = Mod.SaveFunction.Name.Assign( Parts[1] );
						} else {
							bAlreadyAssigned = true;
						}
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".fn-load" ) {
					if( Parts.Num() == 2 ) {
						if( Mod.LoadFunction.Name.IsEmpty() ) {
							bAssignResult = Mod.LoadFunction.Name.Assign( Parts[1] );
						} else {
							bAlreadyAssigned = true;
						}
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".name" ) {
					if( Parts.Num() == 2 ) {
						g_VerboseLog( Filename, uLineNum ) += "Renaming module \"" + Mod.Name + "\" -> \"" + Parts[1] + "\"";
						bAssignResult = Mod.Name.Assign( Parts[1] );
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".object" ) {
					if( Parts.Num() == 2 ) {
						bAssignResult = ObjectFilename.Assign( Parts[1] );
					} else {
						cExpectedArgs = 2;
					}
				} else if( Parts[0] == ".object-dbg" ) {
					if( Parts.Num() == 2 ) {
						bAssignResult = ObjectDbgFilename.Assign( Parts[1] );
					} else {
						cExpectedArgs = 2;
					}
				} else {
					g_ErrorLog( Filename, uLineNum ) += "Unknown directive \"" + Parts[0] + "\"";
					continue;
				}

				if( cExpectedArgs != 0 ) {
					g_ErrorLog( Filename, uLineNum ) += "Directive '" + Parts[0] + "' takes " + Ax::String( ( int )cExpectedArgs - 1 ) + " argument" + ( cExpectedArgs == 2 ? "" : "s" );
					continue;
				}

				if( bAlreadyAssigned ) {
					g_ErrorLog( Filename, uLineNum ) += "Directive '" + Parts[0] + "' was already invoked";
					continue;
				}

				if( !bAssignResult ) {
					g_ErrorLog( Filename, uLineNum ) += "Out of memory for directive '" + Parts[0] + "'";
					continue;
				}

			} else if( IsOldstyleLine( Line ) ) {

				if( !Line.Split( Parts, "%", EKeepEmpty::Yes ) ) {
					g_ErrorLog( Filename, uLineNum ) += "Out of memory from split operation";
					continue;
				}
				if( Parts.Num() < 3 ) {
					g_ErrorLog( Filename, uLineNum ) += "Expected command name, type pattern, and function name for \"" + Line + "\"";
					continue;
				}

				Ax::String &CommandName = Parts[ 0 ];
				Ax::String &TypePattern = Parts[ 1 ];
				Ax::String ReturnType = "0";

				if( CommandName.EndsWith( "[" ) ) {
					CommandName.Remove( -1, 1 );
					if( TypePattern.StartsWith( "I" ) || TypePattern.StartsWith( "U" ) ) {
						ReturnType = TypePattern.Extract( 0, 2 );

						if( ReturnType.Len() < 2 ) {
							g_ErrorLog( Filename, uLineNum ) += "Invalid type sequence for return type (I and U are two-character sequences)";
							continue;
						}
					} else if( TypePattern.StartsWith( "V" ) ) {
						ReturnType = TypePattern.Extract( 0, 3 );

						if( ReturnType.Len() < 3 ) {
							g_ErrorLog( Filename, uLineNum ) += "Invalid type sequence for return type (V is a three-character sequence)";
							continue;
						}
					} else if( TypePattern.StartsWith( "E" ) ) {
						ReturnType = TypePattern.Extract( 0, 4 );

						if( ReturnType.Len() < 4 ) {
							g_ErrorLog( Filename, uLineNum ) += "Invalid type sequence for return type (E is a four-character sequence)";
							continue;
						}
					} else {
						ReturnType = TypePattern.Extract( 0, 1 );
					}
				}

				Ax::String &SymbolName = Parts[ 2 ];

				//g_DebugLog( Filename, uLineNum ) += CommandName + "::" + TypePattern + "::" + ReturnType + "::" + SymbolName;
				
				const SSymbol *const pExistingSym = Mod.Definitions.FindSymbol( CommandName, ESearchArea::ThisScopeOnly );
				SSymbol *const pSym = pExistingSym != nullptr ? const_cast< SSymbol * >( pExistingSym ) : Mod.Definitions.AddSymbol( CommandName );
				if( !pSym ) {
					g_ErrorLog( Filename, uLineNum ) += "Invalid command name: \"" + CommandName + "\"";
					continue;
				}

				if( !pSym->pFunc ) {
					pSym->pFunc = new SFunctionInfo();
					AX_EXPECT_MEMORY( pSym->pFunc );

					pSym->pFunc->Name = CommandName;
				}

				bool bConflicts = false;
				for( const SFunctionOverload &Test : pSym->pFunc->Overloads ) {
					if( Test.ParmTypePattern == TypePattern ) {
						bConflicts = true;
						break;
					}
				}

				if( bConflicts ) {
					g_ErrorLog( Filename, uLineNum ) += "Function with this overload already exists: \"" + CommandName + "\"%\"" + TypePattern + "\"";
					continue;
				}

				Ax::TList< SFunctionOverload >::Iterator OverloadIter = pSym->pFunc->Overloads.AddTail();
				AX_EXPECT_MEMORY( OverloadIter != pSym->pFunc->Overloads.end() );

				SFunctionOverload &Func = *OverloadIter;
				
				if( !ParseTypePattern( Func.Parameters, TypePattern, g_Env->BuildInfo().Platform ) ) {
					g_ErrorLog( Filename, uLineNum ) += "Invalid type pattern: \"" + TypePattern + "\"";
					continue;
				}

				Func.ParmTypePattern.Swap( TypePattern );
				// TypePattern is now invalid

				if( !ParseTypePattern( Func.ReturnInfo, ReturnType, g_Env->BuildInfo().Platform ) ) {
					g_ErrorLog( Filename, uLineNum ) += "Invalid return type pattern: \"" + ReturnType + "\"";
					continue;
				}

				Func.pModule = &Mod;
				Func.RealName.Swap( SymbolName );

				InstallKeyword( CommandName, ( void * )pSym, 0, Ax::Parser::EKeywordExists::Ignore );

			} else if( Line.Contains( "(" ) ) {

				g_ErrorLog( Filename, uLineNum ) += "New-style declarations not yet supported.";
				continue;

			} else {

				g_ErrorLog( Filename, uLineNum ) += "Unknown line in commands file \"" + Line + "\"";
				continue;

			}
		}
		
		if( ObjectFilename.IsEmpty() ) {
			const Ax::String &DLLExt = g_Env->BuildInfo().Platform.DLLExt;

			Ax::String BaseFilename;
			AX_EXPECT_MEMORY( BaseFilename.Assign( ObjectFilename.ExtractDirectory() ) );
			AX_EXPECT_MEMORY( BaseFilename.AppendPath( ObjectFilename.ExtractBasename() ) );

			AX_EXPECT_MEMORY( ObjectFilename.Assign( BaseFilename + DLLExt ) );
			if( ObjectDbgFilename.IsEmpty() ) {
				AX_EXPECT_MEMORY( ObjectDbgFilename.Assign( BaseFilename + "Dbg" + DLLExt ) );
			}
		} else if( ObjectDbgFilename.IsEmpty() ) {
			AX_EXPECT_MEMORY( ObjectDbgFilename.Assign( Filename ) );
		}

		if( ObjectFilename.CaseEndsWith( ".dll" ) || ObjectFilename.CaseEndsWith( ".dylib" ) || ObjectFilename.CaseEndsWith( ".so" ) ) {
			Mod.Type = EModule::DynamicLibrary;
		} else {
			Mod.Type = EModule::StaticLibrary;
		}

		AX_EXPECT_MEMORY( Mod.Filename.Assign( ModDir ) );
		AX_EXPECT_MEMORY( Mod.DebugFilename.Assign( ModDir ) );

		AX_EXPECT_MEMORY( Mod.Filename.AppendPath( ObjectFilename ) );
		AX_EXPECT_MEMORY( Mod.DebugFilename.AppendPath( ObjectDbgFilename ) );

		return &Mod;
	}
	SModule *MModules::LoadFromFile( const Ax::String &Filename )
	{
		Ax::String Text;
		if( !System::ReadFile( Text, Filename ) ) {
			return nullptr;
		}

		return LoadFromText( Filename, Text );
	}
	void MModules::LoadDirectory( const Ax::String &DirectoryName )
	{
		System::CFileList Files;

		if( !System::EnumFileTree( Files, DirectoryName ) ) {
			Ax::Warnf( DirectoryName, "Failed to enumerate directory" );
			return;
		}

		Files.FilterExtension( ".commands" );

		g_VerboseLog( DirectoryName ) += "Found " + Ax::String( ( int )Files.Num() ) + " files";
		size_t cSuccesses = 0;

		Ax::String Filename;
		Ax::String Text;
		for( uintptr i = 0; i < Files.Num(); ++i ) {
			AX_EXPECT_MEMORY( Filename.Assign( Files.GetFile( i ) ) );

			if( !System::ReadFile( Text, Files.GetFile( i ) ) ) {
				continue;
			}

			if( !LoadFromText( Filename, Text ) ) {
				continue;
			}

			++cSuccesses;
		}

		g_VerboseLog( DirectoryName ) += "Successfully loaded " + Ax::String( ( int )cSuccesses ) + " modules";
	}

	const Ax::TList< SModule > &MModules::List() const
	{
		return m_Mods;
	}
	Ax::TList< SModule > &MModules::List()
	{
		return m_Mods;
	}

}}
