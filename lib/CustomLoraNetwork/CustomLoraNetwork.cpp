#include "CustomLoraNetwork.h"
#include "EEPROM.h"
#include "CRC32.h"

CustomLoraNetwork::CustomLoraNetwork(byte M0, byte M1) : CustomLoraNetwork(M0, M1, 0){}

CustomLoraNetwork::CustomLoraNetwork(byte M0, byte M1, uint16_t eepromAddress) : LoraModule(M0, M1){
	this->eepromAddress = eepromAddress;
	hasRecoveredEeprom = false;

	currentNetworkState = IdleNetworkState;
	lastPackageID = UNDEFINEDPACKAGE;
	CustomDataRequested = false;
	CustomDataAvailable = false;

	messageReturnTimeoutInterval = 10000;

	ID = -1;
	parentID = -1;
	lastMainUnitID = -1;

	SendNodeDiscoveryAnswer = false;
	SetNewNodeAddress = false;
	parentAddress = 0;
	nodeAddress = UINT16_MAX - 1; // one less than broadcast address

	nextAvailableNodeAddress = 0;
	nextRelayRequestedAddress = 0;
	nextNotifiedNodeAddress = 0;

	childNodeCount = 0;
	childNodeStartAddress = 1;
	childNodeIDs = nullptr;
	childNodeParentAddresses = nullptr;

	StartRelayDiscovery = false;
	sentDiscoveryRequest = false;
	discoveryTries = 0;
	receivedDiscoveryAnswer = false;
	waitingForRelayDiscovery = false;
	relaysDiscovered = false;

	RegistrationRequested = false;
	RegistrationSuccess = false;

	dataPointerArray = nullptr;
	dataNamesArray = nullptr;
	dataTypesArray = nullptr;

	recDataPointerArray = nullptr;
	recDataNamesArray = nullptr;
	recDataTypesArray = nullptr;

	LoraSetup& setup = LoraModule.setup;

	setup.SetAddress(nodeAddress);
	setup.SetChannel(0);
	setup.SetNetID(0);
	setup.EnableRSSIMeasurement(true);
	setup.EnableAmbientNoiseMeasurement(true);
	setup.EnableLBT(true);
	setup.SetTransmissionMethod(TRANSMISSION_FIXEDPOINT);
	setup.SetAirSpeed(AIRSPEED_300);

	//TODO: Make WorMode Customizable
	setup.SetWORMode(WOR_TRANSMIT);
	setup.SetWORPeriod(WOR_500);
	
	recMessageCounter = 0;
	sentMessageCounter = 0;
}

void CustomLoraNetwork::Init(int maxDataSamples){
	addedSamples = 0;
	this->maxSamples = maxDataSamples;
	this->dataNamesArray = new String[maxDataSamples];
	this->dataTypesArray = new Datatype[maxDataSamples];
	this->dataPointerArray = new void*[maxDataSamples];

	for(int i=0; i<maxDataSamples; i++){
		dataPointerArray[i] = NULL;
	}

	//TODO: Make WorMode Customizable
	LoraModule.Init(false);
	LoraModule.ApplySetup();
}

CustomLoraNetwork::~CustomLoraNetwork(){
	delete [] dataPointerArray;
	delete [] dataNamesArray;
	delete [] dataTypesArray;
	delete [] recDataPointerArray;
	delete [] recDataNamesArray;
	delete [] recDataTypesArray;
}

void CustomLoraNetwork::RecoverEeprom(){
	uint16_t size = 2 * sizeof(uint32_t) + 3* sizeof(long) + 2* sizeof(unsigned short); //24
	if(EEPROM.length() < eepromAddress + size - 1) return;

	uint16_t curAddress = eepromAddress;
	uint32_t checksum;
	EEPROM.get(curAddress,checksum);
	curAddress += sizeof(checksum);

	if(checksum != crc.finalize()) return;

	CRC32 dataCrc;

	long eepromMainUnitID;
	EEPROM.get(curAddress,eepromMainUnitID);
	curAddress += sizeof(eepromMainUnitID);
	dataCrc.update(eepromMainUnitID);

	long eepromID;
	EEPROM.get(curAddress,eepromID);
	curAddress += sizeof(eepromID);
	dataCrc.update(eepromID);

	unsigned short eepromNodeAddress;
	EEPROM.get(curAddress,eepromNodeAddress);
	curAddress += sizeof(eepromNodeAddress);
	dataCrc.update(eepromNodeAddress);

	long eepromParentID;
	EEPROM.get(curAddress,eepromParentID);
	curAddress += sizeof(eepromParentID);
	dataCrc.update(eepromParentID);

	unsigned short eepromParentAddress;
	EEPROM.get(curAddress,eepromParentAddress);
	curAddress += sizeof(eepromParentAddress);
	dataCrc.update(eepromParentAddress);

	EEPROM.get(curAddress,checksum);
	curAddress += sizeof(checksum);

	if(checksum != dataCrc.finalize()) return;

	lastMainUnitID = eepromMainUnitID;
	ID = eepromID;
	nodeAddress = eepromNodeAddress;
	parentID = eepromParentID; 
	parentAddress = eepromParentAddress;
	RegistrationSuccess = true;
}

void CustomLoraNetwork::UpdateEeprom(){
	uint16_t size = 2 * sizeof(uint32_t) + 3* sizeof(long) + 2* sizeof(unsigned short); //24
	if(EEPROM.length() < eepromAddress + size - 1) return;

	uint16_t curAddress = eepromAddress;
	EEPROM.put(curAddress,crc.finalize());
	curAddress += sizeof(uint32_t);

	CRC32 dataCrc;

	EEPROM.put(curAddress,lastMainUnitID);
	curAddress += sizeof(lastMainUnitID);
	dataCrc.update(lastMainUnitID);

	EEPROM.put(curAddress,ID);
	curAddress += sizeof(ID);
	dataCrc.update(ID);

	EEPROM.put(curAddress,nodeAddress);
	curAddress += sizeof(nodeAddress);
	dataCrc.update(nodeAddress);

	EEPROM.put(curAddress,parentID);
	curAddress += sizeof(parentID);
	dataCrc.update(parentID);

	EEPROM.put(curAddress,parentAddress);
	curAddress += sizeof(parentAddress);
	dataCrc.update(parentAddress);

	EEPROM.put(curAddress,dataCrc.finalize());
	curAddress += sizeof(uint32_t);
}

void CustomLoraNetwork::Update(){
	if(!hasRecoveredEeprom){
		hasRecoveredEeprom = true;
		RecoverEeprom();
	}

	LoraModule.Update();

	while(LoraModule.HasPackage()){
		ReadPackage(LoraModule.GetPackage());
		recMessageCounter++;
	}

	switch(currentNetworkState){
		case IdleNetworkState:
			HandleIdleState();
			break;
		case ChildNodeDiscoveryNetworkState:
			HandleChildNodeDiscovery();
			break;
		case ChildRelayDiscoveryNetworkState:
			HandleChildRelayDiscovery();
			break;
		case ParentNodeDiscoveryNetworkState:
			HandleParentNodeDiscovery();
			break;
		case ParentRelayDiscoveryNetworkState:
			HandleParentRelayDiscovery();
			break;
		case RegistrationNetworkState:
			HandleRegistration();
			break;
		case DataAnswerNetworkState:
			HandleDataAnswer();
			break;
	}
}

void CustomLoraNetwork::HandleIdleState(){
	//delay(1000); //Do Nothing
}

void CustomLoraNetwork::HandleChildNodeDiscovery(){
	if(SendNodeDiscoveryAnswer){
		SendNodeDiscoveryAnswer = false;

		SendPackage(parentAddress, PackageType::NetworkNodeDiscoveryAnswer);
	}
	else if (SetNewNodeAddress){
		SetNewNodeAddress = false;

		LoraSetup& setup = LoraModule.setup;
		setup.SetAddress(nodeAddress);
		LoraModule.ApplySetup();
	}
	currentNetworkState = IdleNetworkState;
}

void CustomLoraNetwork::HandleChildRelayDiscovery(){
	if(StartRelayDiscovery){
		StartRelayDiscovery = false;
		currentNetworkState = ParentNodeDiscoveryNetworkState;
	} else {
		SendPackage(parentAddress, PackageType::NetworkRelayDiscoveryAnswer);
		currentNetworkState = IdleNetworkState;
	}
}

void CustomLoraNetwork::HandleParentNodeDiscovery(){
	if(!sentDiscoveryRequest){
		sentDiscoveryRequest = true;
		SendPackage(0xFFFF, PackageType::NetworkNodeDiscoveryRequest);
	} else if(receivedDiscoveryAnswer){
		//Wait at least half the timeout interval for proper responses of other modules
		if(millis() - lastTimePackageSent < messageReturnTimeoutInterval / 2) return;
		receivedDiscoveryAnswer = false;

		for (unsigned short i = nextNotifiedNodeAddress; i < nextAvailableNodeAddress; i++){
			SendPackage(0xFFFF, PackageType::NetworkNodeAddressAssignment);
			nextNotifiedNodeAddress = i + 1;
		}

		sentDiscoveryRequest = false;
	} else if(millis() - lastTimePackageSent > messageReturnTimeoutInterval){
		sentDiscoveryRequest = false;
		if(discoveryTries < 1){
			discoveryTries++;
			return;
		}
		delete [] childNodeParentAddresses;
		if(childNodeCount > 0){
			childNodeParentAddresses = new unsigned short[childNodeCount];
			for(unsigned short i = 0; i< childNodeCount; i++){
				childNodeParentAddresses[i] = nodeAddress;
			}
			currentNetworkState = ParentRelayDiscoveryNetworkState;
		} else {
			//No nodes, so suspend relayDiscovery
			StartRelayDiscovery = false;
			currentNetworkState = ChildRelayDiscoveryNetworkState;
		}
	}
}

void CustomLoraNetwork::HandleParentRelayDiscovery(){
	if(!waitingForRelayDiscovery && nextRelayRequestedAddress < nextNotifiedNodeAddress){
		waitingForRelayDiscovery = true;

		SendPackage(nextRelayRequestedAddress, PackageType::NetworkRelayDiscoveryRequest);
		nextRelayRequestedAddress++;
	} else if(relaysDiscovered){
		relaysDiscovered = false;
		waitingForRelayDiscovery = false;
	} else if(!waitingForRelayDiscovery){
		currentNetworkState = ChildRelayDiscoveryNetworkState;
	}
}

void CustomLoraNetwork::HandleRegistration(){
	if(!RegistrationSuccess){
		SendPackage(0, PackageType::RegistrationAnswer);
	}
	currentNetworkState = IdleNetworkState;
}

void CustomLoraNetwork::HandleDataAnswer(){
	if (!Serial.availableForWrite() || !CustomDataAvailable) return;

	CustomDataAvailable = false;
	CustomDataRequested = false;
	SendPackage(0, PackageType::CustomDataAnswer);
	currentNetworkState = IdleNetworkState;
}

void CustomLoraNetwork::ReadPackage(byte* byteBuffer){
	int offset = 0;

	unsigned short numberOfRelays;
	offset += ReadFromPackage(byteBuffer, offset, &numberOfRelays); 
	if(numberOfRelays > 0){
		RelayPackage(byteBuffer);
		return;
	}

	offset += ReadFromPackage(byteBuffer, offset, &lastPackageID);

	long senderID;
	offset += ReadFromPackage(byteBuffer, offset, &senderID);

	switch (lastPackageID)
	{
		case NetworkNodeDiscoveryRequest:
		{
			if(lastMainUnitID < 0)
			{
				randomSeed(micros()); // Randomize on time of arrival
				ID = random(); 
			}

			long mainUnitID;
			offset += ReadFromPackage(byteBuffer, offset, &mainUnitID);

			if(lastMainUnitID >= 0 && lastMainUnitID == mainUnitID) 
			{
				break;
			}

			offset += ReadFromPackage(byteBuffer, offset, &parentAddress);

			//Reset Node State and send new Discovery Answer
			SendNodeDiscoveryAnswer = true;
			childNodeCount = 0;
			StartRelayDiscovery = false;
			sentDiscoveryRequest = false;
			discoveryTries = 0;
			receivedDiscoveryAnswer = false;
			waitingForRelayDiscovery = false;
			relaysDiscovered = false;
			RegistrationSuccess = false;
			parentID = senderID;
			currentNetworkState = ChildNodeDiscoveryNetworkState;
			break;
		}
		case NetworkNodeDiscoveryAnswer:
		{			
			offset += ReadFromPackage(byteBuffer, offset, &receivedID);
			if(receivedID != ID) return;

			if(childNodeCount <= 0){
				childNodeStartAddress = nextAvailableNodeAddress;
			} else {
				//Check if we already gave that node an address and send it again
				for(unsigned short i = 0; i < childNodeCount; i++){
					if(senderID == childNodeIDs[i]){
						nextNotifiedNodeAddress = childNodeStartAddress + i;
						receivedDiscoveryAnswer = true;
						return;
					}
				}
			}
			nextAvailableNodeAddress += 1;

			//Copy old array then delete
			long* tempChildNodeIDs = childNodeIDs;
			childNodeIDs = new long[childNodeCount + 1];
			for(unsigned short i= 0; i < childNodeCount; i++){
				childNodeIDs[i] = tempChildNodeIDs[i];
			}
			delete [] tempChildNodeIDs;

			childNodeIDs[childNodeCount] = senderID;
			childNodeCount += 1;
			receivedDiscoveryAnswer = true;
		}
		case NetworkNodeAddressAssignment:
		{
			if(senderID != parentID) return;

			offset += ReadFromPackage(byteBuffer, offset, &receivedID);
			if(receivedID != ID) return;

			offset += ReadFromPackage(byteBuffer, offset, &nodeAddress);
			offset += ReadFromPackage(byteBuffer, offset, &lastMainUnitID);
			SetNewNodeAddress = true;
			currentNetworkState = ChildNodeDiscoveryNetworkState;
			break;
		}
		case NetworkRelayDiscoveryRequest:
		{
			if(senderID != parentID) return;

			offset += ReadFromPackage(byteBuffer, offset, &receivedID);
			if(receivedID != ID) return;

			offset += ReadFromPackage(byteBuffer, offset, &nextAvailableNodeAddress);

			nextRelayRequestedAddress = nextAvailableNodeAddress;
			nextNotifiedNodeAddress = nextAvailableNodeAddress;
			StartRelayDiscovery = true;
			currentNetworkState = ChildRelayDiscoveryNetworkState;
			break;
		}
		case NetworkRelayDiscoveryAnswer:
		{
			offset += ReadFromPackage(byteBuffer, offset, &receivedID);
			if(receivedID != ID) return;

			unsigned short numberOfSubNodes;
			offset += ReadFromPackage(byteBuffer, offset, &numberOfSubNodes);

			relaysDiscovered = true;
			if(numberOfSubNodes == 0) return;

			nextAvailableNodeAddress += numberOfSubNodes;

			//Copy old array then delete
			unsigned short* tempNodeAddresses = childNodeParentAddresses;
			childNodeParentAddresses = new unsigned short [childNodeCount + numberOfSubNodes];
			for(unsigned short i= 0; i < childNodeCount; i++){
				childNodeParentAddresses[i] = tempNodeAddresses[i];
			}
			delete [] tempNodeAddresses;

			unsigned short subParentAddress;
			for(unsigned short i = 0; i < numberOfSubNodes; i++){
				offset += ReadFromPackage(byteBuffer, offset, &subParentAddress);
				childNodeParentAddresses[childNodeCount + i] = subParentAddress;
			}

			childNodeCount += numberOfSubNodes;
			break;
		}
		case RegistrationRequest:
		{
			currentNetworkState = NetworkState::RegistrationNetworkState;
			break;
		}
		case RegistrationAnswer:
		{
			offset += ReadFromPackage(byteBuffer, offset, &receivedID);
			offset += ReadFromPackage(byteBuffer, offset, &receivedSamples);

			if(receivedSamples <= 0) return;

			delete [] recDataPointerArray;
			delete [] recDataNamesArray;
			delete [] recDataTypesArray;

			recDataPointerArray = new void*[receivedSamples];
			recDataNamesArray = new String[receivedSamples];
			recDataTypesArray = new Datatype[receivedSamples];

			for(int i=0; i<receivedSamples; i++){
				recDataPointerArray[i] = NULL;
			}

			for(int i = 0; i < receivedSamples; i++){
				offset += ReadFromPackage(byteBuffer, offset, &recDataTypesArray[i]);
				offset += ReadFromPackage(byteBuffer, offset, &recDataNamesArray[i]);
			}
			break;
		}
		case OnRegistrationSuccess:
		{
			offset += ReadFromPackage(byteBuffer, offset, &receivedID);

			if(receivedID == ID){
				RegistrationSuccess = true;
				UpdateEeprom();
			}
			break;
		}
		case DataRequest:
		{
			offset += ReadFromPackage(byteBuffer, offset, &receivedID);

			if(receivedID == ID){
				CustomDataRequested = true;
				currentNetworkState = NetworkState::DataAnswerNetworkState;
			}
			break;
		}
		case CustomDataAnswer:
		{
			offset += ReadFromPackage(byteBuffer, offset, &receivedID);

			if(offset >= (int) LoraModule.GetReceivingPackageSize()) return;

			for(int i = 0; i < receivedSamples; i++){
				offset +=  ReadFromPackage(byteBuffer, offset, &recDataPointerArray[i], recDataTypesArray[i]);
			}
			CustomDataAvailable  = true;
			break;
		}
		default:
			return;
	}
}

void CustomLoraNetwork::SendPackage(unsigned int address, PackageType packageEnum){

	unsigned short numberOfRelays;
	unsigned short mainUnitAddress = 0;
	if(address == mainUnitAddress){
		LoraModule.StartNewSendPackage(parentAddress, 0);
		numberOfRelays = 1;
		AddToPackage(numberOfRelays);
		AddToPackage(mainUnitAddress);//Relay To Address (0 => MainUnit)
	} else {
		LoraModule.StartNewSendPackage(address, 0);
		numberOfRelays = 0;
		AddToPackage(numberOfRelays);
	}
	AddToPackage(packageEnum);
	AddToPackage(ID);

	switch(packageEnum){
		case NetworkNodeDiscoveryRequest:
			AddToPackage(lastMainUnitID);
			AddToPackage(nodeAddress);
			break;

		case NetworkNodeDiscoveryAnswer:
			AddToPackage(parentID);
			break;

		case NetworkNodeAddressAssignment:
			AddToPackage(childNodeIDs[nextNotifiedNodeAddress - childNodeStartAddress]);
			AddToPackage(nextNotifiedNodeAddress);
			AddToPackage(lastMainUnitID);
			break;

		case NetworkRelayDiscoveryRequest:
			AddToPackage(childNodeIDs[nextRelayRequestedAddress - childNodeStartAddress]);
			AddToPackage(nextAvailableNodeAddress);
			break;

		case NetworkRelayDiscoveryAnswer:
			AddToPackage(parentID);
			AddToPackage(childNodeCount);

			for(unsigned short i = 0; i < childNodeCount; i++){
				AddToPackage(childNodeParentAddresses[i]);
			}
			break;

		case RegistrationAnswer:
			AddToPackage(addedSamples);

			for(int i = 0; i < addedSamples; i++){
				AddToPackage(dataTypesArray[i]);
				AddToPackage(dataNamesArray[i]);
			}

			break;

		case CustomDataAnswer:
			for(int i = 0; i < addedSamples; i++){
				AddToPackage(dataPointerArray[i], dataTypesArray[i]);
			}
			break;

		default:
			//Do nothing as we just want an answer
			break;
	};

	LoraModule.SendPackage();
	sentMessageCounter++;
	lastTimePackageSent = millis();
}

void CustomLoraNetwork::RelayPackage(byte* byteBuffer){
	
	int offset = 0;

	unsigned short numberOfRelays;
	offset += ReadFromPackage(byteBuffer, offset, &numberOfRelays); 
	unsigned short nextAddress;
	offset += ReadFromPackage(byteBuffer, offset, &nextAddress);

	unsigned int packageSize = LoraModule.GetReceivingPackageSize();

	if(nextAddress > 0){ 
		//Relay to nextAddress
		LoraModule.StartNewSendPackage(nextAddress, 0);
		numberOfRelays -= 1;
		AddToPackage(numberOfRelays);
	} else {
		//Relay to MainUnit over parentAddress
		LoraModule.StartNewSendPackage(parentAddress, 0);
		offset = 0; //Just copy whole message
	}

	for(int i = offset; i<packageSize; i++){
		AddToPackage(byteBuffer[i]);
	}
	LoraModule.SendPackage();
	sentMessageCounter++;
	lastTimePackageSent = millis();

}

template <typename T>
void CustomLoraNetwork::AddDataPointer(T* Data, String DataName){
	if(addedSamples >= maxSamples) return;

	dataPointerArray[addedSamples] = Data;
	dataNamesArray[addedSamples] = DataName;
	dataTypesArray[addedSamples] = DatatypeConverter::GetDataType<T>(*Data);
	addedSamples++;

	//Update crc
	DataName.getBytes(unsigned char* buf, unsigned int bufsize);
	crc.update(buf, bufsize);
	crc.update(dataTypesArray[addedSamples]);
	crc.update(addedSamples);
}

template void CustomLoraNetwork::AddDataPointer(bool* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(byte* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(unsigned short* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(short* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(unsigned int* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(int* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(unsigned long* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(long* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(float* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(double* Data, String DataName);
template void CustomLoraNetwork::AddDataPointer(String* Data, String DataName);

String CustomLoraNetwork::GetData(int sample){
	if(sample >= addedSamples){
		return "Error: " + String(sample) + ">" + String(addedSamples - 1);
	}
	String name = dataNamesArray[sample];
	Datatype type = dataTypesArray[sample];
	void* pointer = dataPointerArray[sample];

	String DataString = name + " " + type + " ";

	switch (type)
	{
	case Datatype::BOOL:
		DataString += DatatypeConverter::GetDataValue<bool>(pointer);
		break;

	case Datatype::UINT:
		DataString += DatatypeConverter::GetDataValue<unsigned int>(pointer);
		break;

	case Datatype::INT:
		DataString += DatatypeConverter::GetDataValue<int>(pointer);
		break;

	case Datatype::ULONG:
		DataString += DatatypeConverter::GetDataValue<unsigned long>(pointer);
		break;

	case Datatype::LONG:
		DataString += DatatypeConverter::GetDataValue<long>(pointer);
		break;

	case Datatype::FLOAT:
		DataString += DatatypeConverter::GetDataValue<float>(pointer);
		break;

	case Datatype::DOUBLE:
		DataString += DatatypeConverter::GetDataValue<double>(pointer);
		break;

	case Datatype::STRING:
		DataString += DatatypeConverter::GetDataValue<String>(pointer);
		break;

	default:
		DataString += "UNDEFINED VALUE";
		break;
	}

	return DataString;
}

String CustomLoraNetwork::GetRecData(int sample){
	if(sample >= receivedSamples || !RegistrationSuccess){
		return "Error: " + String(sample) + ">" + String(receivedSamples - 1);
	}

	String name = recDataNamesArray[sample];
	Datatype type = recDataTypesArray[sample];
	void* pointer = recDataPointerArray[sample];

	String DataString = name + " " + type + " ";

	if(!CustomDataAvailable){
		return DataString  + "NONE";
	}

	switch (type)
	{
	case Datatype::BOOL:
		DataString += DatatypeConverter::GetDataValue<bool>(pointer);
		break;

	case Datatype::UINT:
		DataString += DatatypeConverter::GetDataValue<unsigned int>(pointer);
		break;

	case Datatype::INT:
		DataString += DatatypeConverter::GetDataValue<int>(pointer);
		break;

	case Datatype::ULONG:
		DataString += DatatypeConverter::GetDataValue<unsigned long>(pointer);
		break;

	case Datatype::LONG:
		DataString += DatatypeConverter::GetDataValue<long>(pointer);
		break;

	case Datatype::FLOAT:
		DataString += DatatypeConverter::GetDataValue<float>(pointer);
		break;

	case Datatype::DOUBLE:
		DataString += DatatypeConverter::GetDataValue<double>(pointer);
		break;

	case Datatype::STRING:
		DataString += DatatypeConverter::GetDataValue<String>(pointer);
		break;

	default:
		DataString += "ERROR";
		break;
	}

	return DataString;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, bool* outData){
	int size = DatatypeConverter::GetSizeOf(BOOL);
	*outData =  DatatypeConverter::GetDataValue<bool>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, PackageType* outData){
	int size = DatatypeConverter::GetSizeOf(BYTE);
	*outData = (PackageType) DatatypeConverter::GetDataValue<byte>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, Datatype* outData){
	int size = DatatypeConverter::GetSizeOf(BYTE);
	*outData = (Datatype) DatatypeConverter::GetDataValue<byte>(&byteBuffer[offset]);

	return size;
}


int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, byte* outData){
	int size = DatatypeConverter::GetSizeOf(BYTE);
	*outData = DatatypeConverter::GetDataValue<byte>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, unsigned int* outData){
	int size = DatatypeConverter::GetSizeOf(UINT);
	*outData = DatatypeConverter::GetDataValue<unsigned int>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, int* outData){
	int size = DatatypeConverter::GetSizeOf(INT);
	*outData = DatatypeConverter::GetDataValue<int>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, unsigned short* outData){
	int size = DatatypeConverter::GetSizeOf(UINT);
	*outData = DatatypeConverter::GetDataValue<unsigned int>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, short* outData){
	int size = DatatypeConverter::GetSizeOf(INT);
	*outData = DatatypeConverter::GetDataValue<int>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, unsigned long* outData){
	int size = DatatypeConverter::GetSizeOf(ULONG);
	*outData = DatatypeConverter::GetDataValue<unsigned long>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, long* outData){
	int size = DatatypeConverter::GetSizeOf(LONG);
	*outData = DatatypeConverter::GetDataValue<long>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, float* outData){
	int size = DatatypeConverter::GetSizeOf(FLOAT);
	*outData = DatatypeConverter::GetDataValue<float>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, double* outData){
	int size = DatatypeConverter::GetSizeOf(DOUBLE);
	*outData = DatatypeConverter::GetDataValue<double>(&byteBuffer[offset]);

	return size;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, String* outData){
	uint8_t stringSize;
	offset += ReadFromPackage(byteBuffer, offset, &stringSize);

	char* buffer = new char[stringSize];
	for (int i=0; i<stringSize; i++){
		buffer[i] = (char)byteBuffer[offset+i];
	}
	*outData = String(buffer);
	delete [] buffer;
	return 1 + stringSize;
}

int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, void** outData, Datatype type){
	int size = 0;

	switch(type){
		case BOOL: {
			size = ReadFromPackage<bool>(byteBuffer, offset, outData);
			break;
		}

		case BYTE: {
			size = ReadFromPackage<byte>(byteBuffer, offset, outData);
			break;
		}

		case UINT: {
			size = ReadFromPackage<unsigned int>(byteBuffer, offset, outData);
			break;
		}
		
		case INT:{
			size = ReadFromPackage<int>(byteBuffer, offset, outData);
			break;
		}

		case ULONG: {
			size = ReadFromPackage<unsigned long>(byteBuffer, offset, outData);
			break;
		}
		
		case LONG:{
			size = ReadFromPackage<long>(byteBuffer, offset, outData);
			break;
		}
		
		case FLOAT:{
			size = ReadFromPackage<float>(byteBuffer, offset, outData);
			break;
		}
		
		case DOUBLE:{
			size = ReadFromPackage<double>(byteBuffer, offset, outData);
			break;
		}
		 
		case STRING:{
			size = ReadFromPackage<String>(byteBuffer, offset, outData);
			break;
		}

		default:
			break;
	}

	return size;
}

template<typename T>
int CustomLoraNetwork::ReadFromPackage(byte* byteBuffer, int offset, void** outData){
	T* data;

	if(*outData != NULL){
		delete static_cast<T*>(*outData);
	}

	data = new T;
	int size = ReadFromPackage(byteBuffer, offset, data);
	*outData = data;

	return size;
}

void CustomLoraNetwork::AddToPackage(bool data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(BOOL));
}

void CustomLoraNetwork::AddToPackage(PackageType data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(BYTE));
}

void CustomLoraNetwork::AddToPackage(Datatype data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(BYTE));
}

void CustomLoraNetwork::AddToPackage(byte data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(BYTE));
}

void CustomLoraNetwork::AddToPackage(unsigned int data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(UINT));
}


void CustomLoraNetwork::AddToPackage(int data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(INT));
}

void CustomLoraNetwork::AddToPackage(unsigned short data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(UINT));
}

void CustomLoraNetwork::AddToPackage(short data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(INT));
}

void CustomLoraNetwork::AddToPackage(unsigned long data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(ULONG));
}

void CustomLoraNetwork::AddToPackage(long data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(LONG));
}

void CustomLoraNetwork::AddToPackage(float data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(FLOAT));
}

void CustomLoraNetwork::AddToPackage(double data){
	byte* byteArray = reinterpret_cast<byte*>(&data);
	LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(DOUBLE));
}

void CustomLoraNetwork::AddToPackage(String data){
	uint8_t size = data.length();
	byte* sizeByte = reinterpret_cast<byte*>(&size);
	LoraModule.AddToSendPackage(sizeByte, sizeof(uint8_t));

	int buffersize = size + 1; // as it needs to contain null termination of string
	unsigned char* buffer = new unsigned char[buffersize];
	data.getBytes(buffer,buffersize);
	LoraModule.AddToSendPackage(reinterpret_cast<byte*>(buffer), size);
	delete [] buffer;
}

void CustomLoraNetwork::AddToPackage(void* data, Datatype type){
	if(type == UNDEFINED) return;

	if(type == STRING){
		AddToPackage(*reinterpret_cast<String*>(data));
	} else {
		byte* byteArray = static_cast<byte*>(data);
		LoraModule.AddToSendPackage(byteArray, DatatypeConverter::GetSizeOf(type));
	}
}

int CustomLoraNetwork::GetLastReceivingPackageSize(){
	return LoraModule.GetReceivingPackageSize();
}

int CustomLoraNetwork::GetLastSendingPackageSize(){
	return LoraModule.GetSendingPackageSize();
}

int CustomLoraNetwork::GetLastRSSI(){
	return LoraModule.GetRSSI();
}

int CustomLoraNetwork::GetLastNoiseStrength(){
	return LoraModule.GetLastNoiseStrength();
}

int CustomLoraNetwork::RequestSNR(){
	return LoraModule.RequestSNR();
}
