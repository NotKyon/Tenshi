#pragma once

#include "../Platform/Platform.hpp"

namespace Ax
{

	template< typename tSingleton >
	class TManager
	{
	public:
		AX_FORCEINLINE tSingleton *operator->()
		{
			return &tSingleton::GetInstance();
		}
		AX_FORCEINLINE const tSingleton *operator->() const
		{
			return &tSingleton::GetInstance();
		}

		AX_FORCEINLINE tSingleton &operator*()
		{
			return tSingleton::GetInstance();
		}
		AX_FORCEINLINE const tSingleton &operator*() const
		{
			return tSingleton::GetInstance();
		}
	};

	//
	//	Example Usage
	//	=============
	//
	//	namespace Ax
	//	{
	//		enum class EFileAccess
	//		{
	//			Read,
	//			Write,
	//			ReadWrite,
	//			Append
	//		};
	//		class MFileSystem
	//		{
	//		public:
	//			static MFileSystem &GetInstance();
	//
	//			class RFile *OpenFile( const char *pszPath, EFileAccess Access );
	//
	//		private:
	//			MFileSystem();
	//		};
	//		static TManager< MFileSystem > FileSystem;
	//
	//		...
	//
	//		RFile *OpenToRead( const char *pszPath )
	//		{
	//			return FileSystem->OpenFile( pszPath, EFileAccess::Read );
	//		}
	//	}
	//

}
