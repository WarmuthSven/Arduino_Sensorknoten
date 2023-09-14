#include "Datatypes.h"

const int datatypesCount = 10;

const int DatatypeConverter::datatypeSizes[datatypesCount] = 
	{
		0,
		sizeof(bool), //1
		sizeof(byte), //1
		sizeof(unsigned int), //2
		sizeof(int), //2
		sizeof(unsigned long), //4
		sizeof(long), //4
		sizeof(float), //4
		sizeof(double), //8
		0 //Variable size
	};

template<typename T>
Datatype DatatypeConverter::GetDataType(T data){
	return Datatype::UNDEFINED;
}

template<>
Datatype DatatypeConverter::GetDataType(bool data){
	return Datatype::BOOL;
}

template<>
Datatype DatatypeConverter::GetDataType(byte data){
	return Datatype::BYTE;
}

template<>
Datatype DatatypeConverter::GetDataType(unsigned short data){
	return Datatype::UINT;
}

template<>
Datatype DatatypeConverter::GetDataType(short data){
	return Datatype::INT;
}

template<>
Datatype DatatypeConverter::GetDataType(unsigned int data){
	return Datatype::UINT;
}

template<>
Datatype DatatypeConverter::GetDataType(int data){
	return Datatype::INT;
}

template<>
Datatype DatatypeConverter::GetDataType(unsigned long data){
	return Datatype::ULONG;
}

template<>
Datatype DatatypeConverter::GetDataType(long data){
	return Datatype::LONG;
}

template<>
Datatype DatatypeConverter::GetDataType(float data){
	return Datatype::FLOAT;
}

// Some Arduinos like the Uno only handle doubles as floats
template<>
Datatype DatatypeConverter::GetDataType(double data){
	if(sizeof(data) < 8ULL){
    	return Datatype::FLOAT;
  	}

  	return Datatype::DOUBLE;
}

template<>
Datatype DatatypeConverter::GetDataType(String data){
	return Datatype::STRING;
}

template<typename T>
T DatatypeConverter::GetDataValue(void* data){
	T* convertedData = static_cast<T*> (data);
	return *convertedData;
}

template<>
bool DatatypeConverter::GetDataValue(void* data){
	bool convertedData;
	memcpy(&convertedData, data, GetSizeOf(BOOL));
	return convertedData;
};

template<>
byte DatatypeConverter::GetDataValue(void* data){
	byte convertedData;
	memcpy(&convertedData, data, GetSizeOf(BYTE));
	return convertedData;
};

template<>
unsigned int DatatypeConverter::GetDataValue(void* data){
	unsigned int convertedData;
	memcpy(&convertedData, data, GetSizeOf(UINT));
	return convertedData;
};

template<>
int DatatypeConverter::GetDataValue(void* data){
	int convertedData;
	memcpy(&convertedData, data, GetSizeOf(INT));
	return convertedData;
};

template<>
unsigned short DatatypeConverter::GetDataValue(void* data){
	unsigned short convertedData;
	memcpy(&convertedData, data, GetSizeOf(UINT));
	return convertedData;
};

template<>
short DatatypeConverter::GetDataValue(void* data){
	short convertedData;
	memcpy(&convertedData, data, GetSizeOf(INT));
	return convertedData;
};

template<>
unsigned long DatatypeConverter::GetDataValue(void* data){
	unsigned long convertedData;
	memcpy(&convertedData, data, GetSizeOf(ULONG));
	return convertedData;
};

template<>
long DatatypeConverter::GetDataValue(void* data){
	long convertedData;
	memcpy(&convertedData, data, GetSizeOf(LONG));
	return convertedData;
};

template<>
float DatatypeConverter::GetDataValue(void* data){
	float convertedData;
	memcpy(&convertedData, data, GetSizeOf(FLOAT));
	return convertedData;
};

template<>
double DatatypeConverter::GetDataValue(void* data){
	double convertedData;
	memcpy(&convertedData, data, GetSizeOf(DOUBLE));
	return convertedData;
};

template String DatatypeConverter::GetDataValue(void* data);

int DatatypeConverter::GetSizeOf(Datatype type){
	if(type < 0 || type >= datatypesCount) return 0;

	return datatypeSizes[type];
}
