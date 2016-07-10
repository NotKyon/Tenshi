#pragma once

#include "../Collections/List.hpp"
#include "../Core/String.hpp"

namespace Ax { namespace Console {

	typedef TList< String >::Iterator ConfigValueIterator;

	struct SConfigLineInfo
	{
		const char *				filename;

		size_t						fileOffset;
		size_t						lineOffset;

		size_t						line;
		size_t						column;

		inline						SConfigLineInfo	()
									: filename( nullptr )
									, fileOffset( 0 )
									, lineOffset( 0 )
									, line( 0 )
									, column( 0 )
									{
									}
	};

	struct SConfigVar
	{
		class Configuration *		pConfig;

		String						name;
		TList< String >				values;

		SConfigVar *				pParent;
		TIntrusiveList< SConfigVar > children;
		TIntrusiveLink< SConfigVar > siblings;

		inline						SConfigVar			()
									: pConfig( nullptr )
									, name()
									, values()
									, pParent( nullptr )
									, siblings( this )
									{
									}
	};

	class Configuration {
	public:
									Configuration		();
									~Configuration		();

		void						Clear				();

		bool						LoadFromMemory		( const char *filename, const char *buffer );
		bool						LoadFromFile		( const char *filename );

		void						PrintVars			();
		void						PrintVar			( const SConfigVar &var );

		SConfigVar *				AddVar				( SConfigVar *parent, const char *name );
		SConfigVar *				FindVar				( SConfigVar *parent, const char *name );
		void						RemoveVar			( SConfigVar *var );

		ConfigValueIterator			AddValue			( SConfigVar *var, const char *value );
		ConfigValueIterator			FindValue			( SConfigVar *var, const char *value );
		void						RemoveValue			( SConfigVar *var, ConfigValueIterator value );
		void						RemoveAllValues		( SConfigVar *var );

		inline SConfigVar *		FirstVar			()
									{
										return m_Sections.Head();
									}
		inline SConfigVar *		LastVar				()
									{
										return m_Sections.Tail();
									}

	private:
		enum class EProcessMode
		{
			// sets the property to a new value if it exists; create if not exist
			// Key=Value
			Set,
			// adds a line to an existing property; create if not exist
			// +Key=Value
			Add,
			// adds a unique line to an existing property; create if not exist
			// .Key=Value
			AddUnique,
			// remove an exact match
			// -Key=Value
			RemoveExact,
			// remove based only on the name
			// !Key=Value
			RemoveInexact
		};

		TIntrusiveList< SConfigVar > m_Sections;
		uintcpu						m_uIncludeDepth;

		void						Process				(
															SConfigVar *parent,
															EProcessMode mode,
															const char *name,
															const char *value,
															const char *filename,
															const char *buffer,
															const char *p
														);

		void						Error				( const SConfigLineInfo &linfo, const char *message );
		void						ErrorRaw			(
															const char *filename,
															const char *buffer,
															const char *p,
															const char *message
														);

		void						Warn				( const SConfigLineInfo &linfo, const char *message );
		void						WarnRaw				(
															const char *filename,
															const char *buffer,
															const char *p,
															const char *message
														);
	};

}}
