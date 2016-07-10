#pragma once

#include <Core/Manager.hpp>

#include "Shell.hpp"
#include "Module.hpp"

namespace Tenshi { namespace Compiler {

	class MBinutils
	{
	public:
		static MBinutils &GetInstance();

		bool Init();
		int Link( const Ax::String &Output, const Ax::TArray< Ax::String > &InObjects, const SModule::IntrList &InMods ) const;

	private:
		MBinutils();
		~MBinutils();

		Ax::String					m_SysRoot;

		Ax::String					m_LD;

		Ax::String					m_LibDir;
		Ax::String					m_IntLibDir;

		Ax::String					m_Obj_CRT2;
		Ax::String					m_Obj_CRTBegin;
		Ax::String					m_Obj_CRTEnd;

		Ax::String					m_Obj_TenshiRuntime;

		AX_DELETE_COPYFUNCS(MBinutils);
	};
	static Ax::TManager<MBinutils>	Binutils;

}}
