#pragma once

#include "../Core/Types.hpp"
#include "../Core/String.hpp"

namespace Ax { namespace System {

	struct SUUID
	{
		uint32 data1;
		uint16 data2;
		uint16 data3;
		uint8  data4[ 8 ];

		String ToString() const;
	};

	void GenerateUuid( SUUID &uuid );

}}
