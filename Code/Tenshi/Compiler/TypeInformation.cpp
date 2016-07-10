#include "_PCH.hpp"
#include "TypeInformation.hpp"
#include "Platform.hpp"
#include "CodeGen.hpp"
#include "Module.hpp"
#include "Project.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	const char *PassByToString( EPassBy PassBy )
	{
		switch( PassBy )
		{
		case EPassBy::Direct:							return "Direct";
		case EPassBy::Reference:						return "Reference";
		case EPassBy::Value:							return "Value";
		}

		AX_ASSERT_MSG( false, "Unreachable" );
		return "(unknown)";
	}
	const char *PassModToString( EPassMod PassMod )
	{
		switch( PassMod )
		{
		case EPassMod::Direct:							return "Direct";
		case EPassMod::ArrayOfZeroOrMoreReferences:		return "ArrayOfZeroOrMoreReferences";
		case EPassMod::ArrayOfOneOrMoreReferences:		return "ArrayOfOneOrMoreReferences";
		case EPassMod::OptionalPointer:					return "OptionalPointer";
		}

		AX_ASSERT_MSG( false, "Unreachable" );
		return "(unknown)";
	}

	Ax::String SMemberInfo::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MSG( Result.Append( Name ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( " as " ), "Out of memory" );

		AX_EXPECT_MSG( Result.Append( Type.ToString() ), "Out of memory" );

		AX_EXPECT_MSG( Result.Append( "[" ), "Out of memory" );

		AX_EXPECT_MSG( Result.Append( "PassBy=" ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( PassByToString( PassBy ) ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( ";" ), "Out of memory" );

		AX_EXPECT_MSG( Result.Append( "PassMod=" ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( PassModToString( PassMod ) ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( ";" ), "Out of memory" );

		AX_EXPECT_MSG( Result.Append( "]" ), "Out of memory" );

		AX_EXPECT_MSG( Result.Append( "@" ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( Ax::String::FromUnsignedInteger( uOffset ) ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( "x" ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( Ax::String::FromUnsignedInteger( Type.cBytes ) ), "Out of memory" );

		return Result;
	}
	Ax::String STypeInfo::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MSG( Result.Append( "Type " ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( Name ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( " (" ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( Ax::String::FromUnsignedInteger( Members.Num() ) ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( ")" ), "Out of memory" );

		AX_EXPECT_MSG( Result.Append( " @" ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( Ax::String::FromUnsignedInteger( uAlignment ) ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( "x" ), "Out of memory" );
		AX_EXPECT_MSG( Result.Append( Ax::String::FromUnsignedInteger( cBytes ) ), "Out of memory" );

		if( bIsInitTrivial ) {
			AX_EXPECT_MSG( Result.Append( " +trivinit" ), "Out of memory" );
		}
		if( bIsFiniTrivial ) {
			AX_EXPECT_MSG( Result.Append( " +trivfini" ), "Out of memory" );
		}
		if( bIsCopyTrivial ) {
			AX_EXPECT_MSG( Result.Append( " +trivcopy" ), "Out of memory" );
		}
		if( bIsMoveTrivial ) {
			AX_EXPECT_MSG( Result.Append( " +trivmove" ), "Out of memory" );
		}

		AX_EXPECT_MSG( Result.Append( "\n{\n" ), "Out of memory" );

		for( const SMemberInfo &Info : Members ) {
			AX_EXPECT_MSG( Result.Append( "\t" ), "Out of memory" );
			AX_EXPECT_MSG( Result.Append( Info.ToString() ), "Out of memory" );
			AX_EXPECT_MSG( Result.Append( "\n" ), "Out of memory" );
		}
		AX_EXPECT_MSG( Result.Append( "}" ), "Out of memory" );

		return Result;
	}

	bool SFunctionOverload::GenDecl()
	{
		if( pLLVMFunc != nullptr ) {
			return true;
		}

		AX_EXPECT_MSG( LLVMTypes.Reserve( Parameters.Num() ), "Out of memory" );

		for( SMemberInfo &Parm : Parameters ) {
#if 1
			llvm::Type *const pPreParmTy = Parm.Type.CodeGen();
			if( !pPreParmTy ) {
				return false;
			}
#else
			llvm::Type *const pPreParmTy = Parm.Type.GetParameterType();
			if( !pPreParmTy ) {
				return false;
			}
#endif

			llvm::Type *const pParmTy =
				( Parm.PassBy == EPassBy::Reference ) ? (
					pPreParmTy->getPointerTo()
				) : (
					pPreParmTy
				);

			LLVMTypes.Append( pParmTy );
		}
		AX_ASSERT( LLVMTypes.Num() == Parameters.Num() );

		pLLVMReturnType = ReturnInfo.Type.CodeGen();
		if( !pLLVMReturnType ) {
			return false;
		}

		pLLVMFuncType =
			llvm::FunctionType::get
			(
				pLLVMReturnType,
				llvm::ArrayRef< llvm::Type * >
				(
					LLVMTypes.begin(),
					LLVMTypes.end()
				),
				false
			);
		if( !pLLVMFuncType ) {
			return false;
		}

		llvm::GlobalValue::LinkageTypes Linkage = llvm::GlobalValue::LinkageTypes::ExternalLinkage;

		if( pModule != nullptr ) {
			switch( pModule->Type ) {
			case EModule::Internal:
			case EModule::DynamicLibrary:
			case EModule::StaticLibrary:
				Linkage = llvm::GlobalValue::LinkageTypes::AvailableExternallyLinkage;
				break;
			case EModule::UserCode:
				break;
			}
		}

		pLLVMFunc = llvm::Function::Create( pLLVMFuncType, Linkage, LLVMStr( RealName ), &CG->Module() );
		if( !pLLVMFunc ) {
			return false;
		}

		if( pModule != nullptr ) {
			if( pModule->Type == EModule::DynamicLibrary ) {
				pLLVMFunc->setDLLStorageClass( llvm::GlobalValue::DLLImportStorageClass );
			}

			Projects->Current().TouchModule( *pModule );
		}

		return true;
	}

	SFunctionOverload *SFunctionInfo::FindMatch( const Ax::TArray< SExpr > &Subexpressions, Ax::TArray< ECast > &OutCasts ) const
	{
		static const Ax::uintptr kMaxOverloads = 32;
		SFunctionOverload *pOverloads[ kMaxOverloads ];
		Ax::uintptr cOverloads = 0;
		SFunctionOverload *pBest = nullptr;
		Ax::uintptr cBestCasts = ~Ax::uintptr( 0 );
		static const Ax::uintptr kMaxArgs = 64;
		ECast ArgCasts[ kMaxArgs ];

		AX_EXPECT_MSG( Subexpressions.Num() < kMaxArgs, "Too many arguments" );

		AX_EXPECT_MSG( OutCasts.Resize( Subexpressions.Num() ), "Out of memory" );

		// Find all overloads with the same number of parameters
		for( SFunctionOverload &Overload : Overloads ) {
			AX_ASSERT( Overload.Parameters.Num() <= kMaxArgs );

			// FIXME: Check for default parameters
			if( Overload.Parameters.Num() != Subexpressions.Num() ) {
				continue;
			}

			// Add the overload to the array of candidates
			if( cOverloads == kMaxOverloads ) {
				// FIXME: Error...
				continue;
			}

			pOverloads[ cOverloads++ ] = &Overload;
		}

		// For each of the found overloads
		for( Ax::uintptr i = 0; i < cOverloads; ++i ) {
			AX_ASSERT( i < kMaxOverloads );
			AX_ASSERT_NOT_NULL( pOverloads[ i ] );
			SFunctionOverload &Overload = *pOverloads[ i ];

			// Check how many casts are needed
			Ax::uintptr cCasts = 0;
			Ax::uintptr uParmI = 0;
			bool bNotMatched = false;
			for( const CExpression *pSubexpr : Subexpressions ) {
				AX_ASSERT_NOT_NULL( pSubexpr );
				AX_ASSERT( uParmI < Overload.Parameters.Num() );

				const SMemberInfo &Parm = Overload.Parameters.At( uParmI );

				const STypeRef *const pType = pSubexpr->GetType();
				if( !pType ) {
					bNotMatched = true;
					break;
				}

				const ECast CastOp = STypeRef::Cast( *pType, Parm.Type );
				if( CastOp == ECast::Invalid ) {
					bNotMatched = true;
					break;
				}

				ArgCasts[ uParmI ] = CastOp;

				if( CastOp != ECast::None ) {
					++cCasts;
				}

				++uParmI;
			}

			if( !bNotMatched && cCasts < cBestCasts ) {
				pBest = &Overload;
				cBestCasts = cCasts;
				for( Ax::uintptr j = 0; j < Subexpressions.Num(); ++j ) {
					OutCasts[ j ] = ArgCasts[ j ];
				}
			}
		}

		return pBest;
	}
	bool SFunctionInfo::GenDecls()
	{
		for( SFunctionOverload &Func : Overloads ) {
			if( !Func.GenDecl() ) {
				return false;
			}
		}

		return true;
	}

	Ax::String GetTypePattern( const Ax::TArray< SMemberInfo > &InMembers )
	{
		Ax::String TypePattern;

		AX_EXPECT_MSG( TypePattern.Reserve( InMembers.Num()*4 ), "Out of memory" );
		for( const SMemberInfo &Member : InMembers ) {
			char szBuf[ 8 ];
			uintptr i = 0;

#define W1(X_)						szBuf[ i++ ] = X_; break
#define W2(X_,Y_)					szBuf[ i++ ] = X_; szBuf[ i++ ] = Y_; break
#define W3(X_,Y_,Z_)				szBuf[ i++ ] = X_; szBuf[ i++ ] = Y_; szBuf[ i++ ] = Z_; break
#define W4(X_,Y_,Z_,W_)				szBuf[ i++ ] = X_; szBuf[ i++ ] = Y_; szBuf[ i++ ] = Z_; szBuf[ i++ ] = W_; break

			switch( Member.Type.BuiltinType ) {
			case EBuiltinType::Any:						W1('X');
			case EBuiltinType::ArrayObject:				W1('H');
			case EBuiltinType::BinaryTreeObject:		W1('M');
			case EBuiltinType::Boolean:					W1('B');
			case EBuiltinType::ConstUTF8Pointer:		W1('S');
			case EBuiltinType::ConstUTF16Pointer:		W1('T');
			case EBuiltinType::Float16:					W1('A');
			case EBuiltinType::Float32:					W1('F');
			case EBuiltinType::Float64:					W1('O');
			case EBuiltinType::Int8:					W1('C');
			case EBuiltinType::Int16:					W1('N');
			case EBuiltinType::Int32:					W1('L');
			case EBuiltinType::Int64:					W1('R');
			case EBuiltinType::Int128:					W4('E','I','1','6');
			case EBuiltinType::LinkedListObject:		W1('K');
			case EBuiltinType::SIMDVector128:			W3('V','4','X');
			case EBuiltinType::SIMDVector128_4Float32:	W3('V','4','F');
			case EBuiltinType::SIMDVector128_4Int32:	W3('V','4','L');
			case EBuiltinType::SIMDVector128_2Float64:	W3('V','2','O');
			case EBuiltinType::SIMDVector128_2Int64:	W3('V','2','R');
			case EBuiltinType::SIMDVector256:			W3('V','8','X');
			case EBuiltinType::SIMDVector256_8Float32:	W3('V','8','F');
			case EBuiltinType::SIMDVector256_8Int32:	W3('V','8','L');
			case EBuiltinType::SIMDVector256_4Float64:	W3('V','4','O');
			case EBuiltinType::SIMDVector256_4Int64:	W3('V','4','R');
			case EBuiltinType::SNorm8:					W4('E','S','N','1');
			case EBuiltinType::SNorm16:					W4('E','S','N','2');
			case EBuiltinType::SNorm32:					W4('E','S','N','4');
			case EBuiltinType::SNorm64:					W4('E','S','N','8');
			case EBuiltinType::StringObject:			W1('G');
			case EBuiltinType::UInt8:					W1('Y');
			case EBuiltinType::UInt16:					W1('W');
			case EBuiltinType::UInt32:					W1('D');
			case EBuiltinType::UInt64:					W1('Q');
			case EBuiltinType::UInt128:					W4('E','U','1','6');
			case EBuiltinType::UNorm8:					W4('E','U','N','1');
			case EBuiltinType::UNorm16:					W4('E','U','N','2');
			case EBuiltinType::UNorm32:					W4('E','U','N','4');
			case EBuiltinType::UNorm64:					W4('E','U','N','8');

			case EBuiltinType::Vector2f:				W4('E','F','2','1');
			case EBuiltinType::Vector3f:				W4('E','F','3','1');
			case EBuiltinType::Vector4f:				W4('E','F','4','1');
			
			case EBuiltinType::Matrix2f:				W4('E','F','2','2');
			case EBuiltinType::Matrix3f:				W4('E','F','3','3');
			case EBuiltinType::Matrix4f:				W4('E','F','4','4');
			case EBuiltinType::Matrix23f:				W4('E','F','2','3');
			case EBuiltinType::Matrix24f:				W4('E','F','2','4');
			case EBuiltinType::Matrix32f:				W4('E','F','3','2');
			case EBuiltinType::Matrix34f:				W4('E','F','3','4');
			case EBuiltinType::Matrix42f:				W4('E','F','4','2');
			case EBuiltinType::Matrix43f:				W4('E','F','4','3');

			case EBuiltinType::Quaternionf:				W4('E','F','4','Q');

			case EBuiltinType::UserDefined:
				{
					Ax::String Subpattern;

					AX_ASSERT_NOT_NULL( Member.Type.pCustomType );
					AX_EXPECT_MSG( Subpattern.Assign( GetTypePattern( Member.Type.pCustomType->Members ) ), "Out of memory" );

					AX_EXPECT_MSG( TypePattern.Reserve( TypePattern.NumAllocated() + Subpattern.Len() + 3 ), "Out of memory" );
					AX_EXPECT_MSG( TypePattern.Append( "X(" ), "Out of memory" );
					AX_EXPECT_MSG( TypePattern.Append( Subpattern ), "Out of memory" );
					AX_EXPECT_MSG( TypePattern.Append( ")" ), "Out of memory" );
				}
				break;
			case EBuiltinType::Void:					W1('0');
			}
			
			AX_ASSERT( i < sizeof( szBuf ) );
			if( Member.PassBy == EPassBy::Reference ) {
				switch( Member.Type.Access ) {
				case EAccess::ReadOnly:					W1('<');
				case EAccess::WriteOnly:				W1('>');
				case EAccess::ReadWrite:				W1('*');
				}
			}
			
			AX_ASSERT( i < sizeof( szBuf ) );
			switch( Member.PassMod ) {
			case EPassMod::Direct:						break;
			case EPassMod::ArrayOfZeroOrMoreReferences:	W1('~');
			case EPassMod::ArrayOfOneOrMoreReferences:	W1('+');
			case EPassMod::OptionalPointer:				W1('?');
			}

			AX_ASSERT( i < sizeof( szBuf ) );
			szBuf[ i ] = '\0';

			AX_EXPECT_MSG( TypePattern.Append( szBuf ), "Out of memory" );

#undef W4
#undef W3
#undef W2
#undef W1
		}

		return TypePattern;
	}
	const char *ParseTypePatternPart( SMemberInfo &OutMember, const char *pszPattern, const SPlatform &Platform )
	{
		AX_ASSERT_NOT_NULL( pszPattern );

		SMemberInfo &Info = OutMember;
		const char *p = pszPattern;
			
		Info.Type.BuiltinType = EBuiltinType::Invalid;

		if( *p == '\0' ) {
			return p;
		}

#define W(Letter_,Type_) case Letter_: Info.Type.BuiltinType = EBuiltinType::Type_; ++p; break
		switch( *p ) {
			W('0',Void);
			W('A',Float16);
			W('B',Boolean);
			W('C',Int8);
			W('D',UInt32);
			W('F',Float32);
			W('G',StringObject);
			W('H',ArrayObject);
			W('K',LinkedListObject);
			W('L',Int32);
			W('M',BinaryTreeObject);
			W('N',Int16);
			W('O',Float64);
			W('Q',UInt64);
			W('R',Int64);
			W('S',ConstUTF8Pointer);
			W('T',ConstUTF16Pointer);
			W('W',UInt16);
			W('X',Any);
			W('Y',UInt8);

		case 'E':
			if( p[1]=='I' && p[2]=='1' && p[3]=='6' ) {
				Info.Type.BuiltinType = EBuiltinType::Int128;
			} else if( p[1] == 'U' && p[2]=='1' && p[3]=='6' ) {
				Info.Type.BuiltinType = EBuiltinType::UInt128;
			} else if( p[1] == 'S' && p[2]=='N' ) {
				if( p[3]=='1' ) {
					Info.Type.BuiltinType = EBuiltinType::SNorm8;
				} else if( p[3]=='2' ) {
					Info.Type.BuiltinType = EBuiltinType::SNorm16;
				} else if( p[3]=='4' ) {
					Info.Type.BuiltinType = EBuiltinType::SNorm32;
				} else if( p[3]=='8' ) {
					Info.Type.BuiltinType = EBuiltinType::SNorm64;
				}
			} else if( p[1] == 'U' && p[2]=='N' ) {
				if( p[3]=='1' ) {
					Info.Type.BuiltinType = EBuiltinType::UNorm8;
				} else if( p[3]=='2' ) {
					Info.Type.BuiltinType = EBuiltinType::UNorm16;
				} else if( p[3]=='4' ) {
					Info.Type.BuiltinType = EBuiltinType::UNorm32;
				} else if( p[3]=='8' ) {
					Info.Type.BuiltinType = EBuiltinType::UNorm64;
				}
			} else if( p[1] == 'F' ) {
				if( p[2]=='2' ) {
					if( p[3]=='1' ) {
						Info.Type.BuiltinType = EBuiltinType::Vector2f;
					} else if( p[3]=='2' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix2f;
					} else if( p[3]=='3' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix23f;
					} else if( p[3]=='4' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix24f;
					}
				} else if( p[2]=='3' ) {
					if( p[3]=='1' ) {
						Info.Type.BuiltinType = EBuiltinType::Vector3f;
					} else if( p[3]=='2' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix32f;
					} else if( p[3]=='3' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix3f;
					} else if( p[3]=='4' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix34f;
					}
				} else if( p[2]=='4' ) {
					if( p[3]=='1' ) {
						Info.Type.BuiltinType = EBuiltinType::Vector4f;
					} else if( p[3]=='2' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix42f;
					} else if( p[3]=='3' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix43f;
					} else if( p[3]=='4' ) {
						Info.Type.BuiltinType = EBuiltinType::Matrix4f;
					} else if( p[3]=='Q' ) {
						Info.Type.BuiltinType = EBuiltinType::Quaternionf;
					}
				}
			}
			if( Info.Type.BuiltinType == EBuiltinType::Invalid ) {
				return nullptr;
			}
			p += 4;
			break;
		case 'I':
			if( p[1]=='1' ) {
				Info.Type.BuiltinType = EBuiltinType::Int8;
			} else if( p[1]=='2' ) {
				Info.Type.BuiltinType = EBuiltinType::Int16;
			} else if( p[1]=='4' ) {
				Info.Type.BuiltinType = EBuiltinType::Int32;
			} else if( p[1]=='8' ) {
				Info.Type.BuiltinType = EBuiltinType::Int64;
			} else if( p[1]=='P' ) {
				switch( Platform.PointerSize ) {
				case kPointer32:
					Info.Type.BuiltinType = EBuiltinType::Int32;
					break;
				default:
					Info.Type.BuiltinType = EBuiltinType::Int64;
					break;
				}
			} else {
				return nullptr;
			}
			p += 2;
			break;
		case 'U':
			if( p[1]=='1' ) {
				Info.Type.BuiltinType = EBuiltinType::UInt8;
			} else if( p[1]=='2' ) {
				Info.Type.BuiltinType = EBuiltinType::UInt16;
			} else if( p[1]=='4' ) {
				Info.Type.BuiltinType = EBuiltinType::UInt32;
			} else if( p[1]=='8' ) {
				Info.Type.BuiltinType = EBuiltinType::UInt64;
			} else if( p[1]=='P' ) {
				switch( Platform.PointerSize ) {
				case kPointer32:
					Info.Type.BuiltinType = EBuiltinType::UInt32;
					break;
				default:
					Info.Type.BuiltinType = EBuiltinType::UInt64;
					break;
				}
			} else {
				return nullptr;
			}
			p += 2;
			break;
		case 'V':
			if( p[1]=='2' ) {
				if( p[2]=='O' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector128_2Float64;
				} else if( p[2]=='R' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector128_2Int64;
				} else {
					return nullptr;
				}
			} else if( p[1]=='4' ) {
				if( p[2]=='X' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector128;
				} else if( p[2]=='F' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector128_4Float32;
				} else if( p[2]=='L' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector128_4Int32;
				} else if( p[2]=='O' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector256_4Float64;
				} else if( p[2]=='R' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector256_4Int64;
				} else {
					return nullptr;
				}
			} else if( p[1]=='8' ) {
				if( p[2]=='X' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector256;
				} else if( p[2]=='F' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector256_8Float32;
				} else if( p[2]=='L' ) {
					Info.Type.BuiltinType = EBuiltinType::SIMDVector256_8Int32;
				} else {
					return nullptr;
				}
			} else {
				return nullptr;
			}
			p += 3;
			break;

		case '#':
			if( p[1]=='2' ) {
				Info.Type.BuiltinType = EBuiltinType::Float16;
			} else if( p[1]=='4' ) {
				Info.Type.BuiltinType = EBuiltinType::Float32;
			} else if( p[1]=='8' ) {
				Info.Type.BuiltinType = EBuiltinType::Float64;
			} else {
				return nullptr;
			}
			p += 2;
			break;

		default:
			return nullptr;
		}

		if( Info.Type.BuiltinType == EBuiltinType::Invalid ) {
			return nullptr;
		}

		Info.PassBy = EPassBy::Value;
		if( Info.Type.BuiltinType == EBuiltinType::ArrayObject ) {
			Info.PassBy = EPassBy::Reference;
		}

		Info.Type.Access = EAccess::ReadOnly;
		if( *p == '<' ) {
			Info.PassBy = EPassBy::Reference;
			++p;
		} else if( *p == '>' ) {
			Info.PassBy = EPassBy::Reference;
			Info.Type.Access = EAccess::WriteOnly;
			++p;
		} else if( *p == '*' ) {
			Info.PassBy = EPassBy::Reference;
			Info.Type.Access = EAccess::ReadWrite;
			++p;
		}

		Info.PassMod = EPassMod::Direct;
		if( *p == '~' ) {
			Info.PassMod = EPassMod::ArrayOfZeroOrMoreReferences;
			++p;
		} else if( *p == '+' ) {
			Info.PassMod = EPassMod::ArrayOfOneOrMoreReferences;
			++p;
		} else if( *p == '?' ) {
			Info.PassMod = EPassMod::OptionalPointer;
			++p;
		}

		Info.Type.pCustomType = nullptr;
		Info.Type.cBytes = GetTypeSize( Info.Type.BuiltinType );
		Info.uOffset = 0;
		return p;
	}
	bool ParseTypePattern( Ax::TArray< SMemberInfo > &OutMembers, const char *pszPattern, const SPlatform &Platform )
	{
		AX_ASSERT_NOT_NULL( pszPattern );

		if( *pszPattern == '\0' ) {
			return true;
		}

		AX_EXPECT_MSG( OutMembers.Reserve( strlen( pszPattern ) ), "Out of memory" );

		Ax::uint32 uOffset = 0;

		const char *p = pszPattern;
		while( *p != '\0' ) {
			SMemberInfo Info;

			p = ParseTypePatternPart( Info, p, Platform );
			if( !p ) {
				return false;
			}

			Info.uOffset = uOffset;

			uOffset += Info.Type.cBytes;
			if( uOffset%Platform.TypeMemberAlignment != 0 ) {
				uOffset += Platform.TypeMemberAlignment;
				uOffset -= uOffset%Platform.TypeMemberAlignment;
			}

			AX_EXPECT_MSG( OutMembers.Append( Info ), "Out of memory" );
		}

		return true;
	}
	bool ParseTypePattern( SMemberInfo &OutMember, const char *pszPattern, const SPlatform &Platform )
	{
		if( !pszPattern || *pszPattern == '\0' || ( pszPattern[0]=='0' && pszPattern[1]=='\0' ) ) {
			OutMember.PassBy = EPassBy::Direct;
			OutMember.PassMod = EPassMod::Direct;
			OutMember.uOffset = 0;
			OutMember.Type.BuiltinType = EBuiltinType::Void;
			return true;
		}

		return ParseTypePatternPart( OutMember, pszPattern, Platform ) != nullptr;
	}

}}
