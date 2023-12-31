#ifndef CUSTOMLORANETWORK_H
#define CUSTOMLORANETWORK_H

#include "Arduino.h"
#include "Lora.h"
#include "Datatypes.h"
#include "CRC32.h"

enum PackageType : byte{
	NetworkNodeDiscoveryRequest = 0,
	NetworkNodeDiscoveryAnswer = 1,
	NetworkNodeAddressAssignment = 2,
	NetworkRelayDiscoveryRequest = 3,
	NetworkRelayDiscoveryAnswer = 4,
	RegistrationRequest = 5,
	RegistrationAnswer = 6,
	OnRegistrationSuccess = 7,
	DataRequest = 8,
	CustomDataAnswer = 9,
	UNDEFINEDPACKAGE = UINT8_MAX
};



class CustomLoraNetwork{
	private:
		enum NetworkState : byte{
			IdleNetworkState,
			ChildNodeDiscoveryNetworkState,
			ChildRelayDiscoveryNetworkState,
			ParentNodeDiscoveryNetworkState,
			ParentRelayDiscoveryNetworkState,
			RegistrationNetworkState,
			DataAnswerNetworkState
		};

		Lora LoraModule;
		uint16_t eepromAddress;
		bool hasRecoveredEeprom;

		CRC32 crc;
		uint32_t setupChecksum;

		NetworkState currentNetworkState;
		PackageType lastPackageID;

		long ID;
		long parentID;
		long lastMainUnitID;
		long receivedID;

		int maxSamples;
		int addedSamples;
		int receivedSamples;
		void** dataPointerArray;
		String* dataNamesArray;
		Datatype* dataTypesArray;

		void** recDataPointerArray;
		String* recDataNamesArray;
		Datatype* recDataTypesArray;

		unsigned long messageReturnTimeoutInterval;
		unsigned long lastTimePackageSent;
		unsigned long recMessageCounter;
		unsigned long sentMessageCounter;

		bool SendNodeDiscoveryAnswer;
		bool SetNewNodeAddress;
		unsigned short nodeAddress;
		unsigned short parentAddress;
	
		unsigned short nextAvailableNodeAddress;
		unsigned short nextNotifiedNodeAddress;
		unsigned short nextRelayRequestedAddress;

		unsigned short childNodeCount;
		unsigned short childNodeStartAddress;
		unsigned short* childNodeParentAddresses;
		long* childNodeIDs;

		bool StartRelayDiscovery;
		bool sentDiscoveryRequest;
		uint8_t discoveryTries;
		bool receivedDiscoveryAnswer;
		bool waitingForRelayDiscovery;
		bool relaysDiscovered;

		bool RegistrationRequested;
		bool RegistrationSuccess;
		bool CustomDataRequested;
		bool CustomDataAvailable;
		
		void RecoverEeprom();
		void UpdateEeprom();

		void HandleIdleState();
		void HandleChildNodeDiscovery();
		void HandleChildRelayDiscovery();
		void HandleParentNodeDiscovery();
		void HandleParentRelayDiscovery();
		void HandleRegistration();
		void HandleDataAnswer();

		void ReadPackage(byte* byteBuffer);
		void SendPackage(unsigned int address, PackageType packageEnum);
		void RelayPackage(byte* byteBuffer);

		int ReadFromPackage(byte* byteBuffer, int offset, bool* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, PackageType* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, Datatype* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, byte* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, unsigned int* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, int* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, unsigned short* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, short* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, unsigned long* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, long* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, float* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, double* outData);
		int ReadFromPackage(byte* byteBuffer, int offset, String* outData);
		template<typename T>
		int ReadFromPackage(byte* byteBuffer, int offset, void** outData);
		int ReadFromPackage(byte* byteBuffer, int offset, void** outData, Datatype type);

		void AddToPackage(bool data);
		void AddToPackage(PackageType data);
		void AddToPackage(Datatype data);
		void AddToPackage(byte data);
		void AddToPackage(unsigned int data);
		void AddToPackage(int data);
		void AddToPackage(unsigned short data);
		void AddToPackage(short data);
		void AddToPackage(unsigned long data);
		void AddToPackage(long data);
		void AddToPackage(float data);
		void AddToPackage(double data);
		void AddToPackage(String data);
		void AddToPackage(void* data, Datatype type);

	public:
		CustomLoraNetwork(byte M0, byte M1);
		CustomLoraNetwork(byte M0, byte M1, uint16_t eepromAddress);
		~CustomLoraNetwork();
		void Init(int maxDataSamples);
		void Update();
		template <typename T>
		void AddDataPointer(T* Data, String DataName);
		String GetData(int sample);
		String GetRecData(int sample);

		int GetLastReceivingPackageSize();
		int GetLastSendingPackageSize();

		int GetLastRSSI();
		int GetLastNoiseStrength();
		int RequestSNR();

		bool NeedCustomDataUpdate();
		void ConfirmCustomDataUpdate();
};


#endif
