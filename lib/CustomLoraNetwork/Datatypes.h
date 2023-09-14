#ifndef DATATYPES_H
#define DATATYPES_H

#include "Arduino.h"

enum Datatype : byte{
	UNDEFINED = 0,
	BOOL = 1,
	BYTE = 2,
	UINT = 3,
	INT = 4,
	ULONG = 5,
	LONG = 6,
	FLOAT = 7,
	DOUBLE = 8, 
	STRING = 9
};

class DatatypeConverter{
	private:
		const static int datatypeSizes[10];
		
	public:
		template<typename T>
		static Datatype GetDataType(T data);

		template<typename T>
		static T GetDataValue(void* data);

		static int GetSizeOf(Datatype type);
};

#endif
