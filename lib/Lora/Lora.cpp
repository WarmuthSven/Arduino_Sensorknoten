#include "Lora.h"

Lora::Lora(byte M0, byte M1){
	this->M0 = M0;
	this->M1 = M1;
}

void Lora::Init(bool worMode){
	this->worMode = worMode;

	pinMode(M0, OUTPUT);
	pinMode(M1, OUTPUT);
	if(worMode){
		digitalWrite(M0, HIGH);
		digitalWrite(M1, LOW);
	} else{
		digitalWrite(M0, LOW);
		digitalWrite(M1, LOW);
	}
}

void Lora::Update(){

}

void Lora::ApplySetup(){
	digitalWrite(M0, LOW);
	digitalWrite(M1, HIGH);
	delay(100);

	byte* data = setup.ReturnByteArray();

	bool setupComplete = false;
	unsigned long startTime = 0;

	//Wait for answer of Lora-Chip else repeat
	while(!setupComplete){
		Serial.write(data, 12);

		startTime = millis();
		while(!setupComplete && millis() - startTime < Serial.getTimeout()){
			setupComplete = Serial.available();
		}
		delay(100);
	}

	//TODO: Check if Answer has correct format
	while(Serial.available()){
		Serial.read();
	}
	
	if(worMode){
		digitalWrite(M0, HIGH);
		digitalWrite(M1, LOW);
	} else{
		digitalWrite(M0, LOW);
		digitalWrite(M1, LOW);
	}
	delay(100);
}

void Lora::StartNewSendPackage(uint8_t estimatedDataSize = 0){
	sendingPackage.ResetBytes(estimatedDataSize,1);
}

void Lora::StartNewSendPackage(unsigned int address, uint8_t channel, uint8_t estimatedDataSize = 0){
	if(channel > 83){
		channel = 83;
	}

	sendingPackage.ResetBytes(estimatedDataSize,4);
	sendingPackage.SetHeader(address, channel);
}

void Lora::AddToSendPackage(byte* bytes, int size){
	if(size <= 0) return;

	sendingPackage.WriteBytes(bytes, size);
}

void Lora::SendPackage(){
	sendingPackage.Send();
}

bool receiveOnce = true;

bool Lora::HasPackage(){
	if(Serial.available()){
		receivingPackage.ReadBytes(setup.enableRSSI);

		return true;
	}

	return false;
}

uint8_t Lora::Package::combinedOffset(){
	return headerOffset + dataOffset;
}

void Lora::Package::ReadBytes(bool receiveRSSI){
	byte bytes[1];
	Serial.readBytes(bytes, 0);

	capacity = DatatypeConverter::GetDataValue<byte>(bytes);
	headerOffset = 0;
	dataOffset = capacity;

	delete [] byteBuffer;
	byteBuffer = new byte[capacity];

	Serial.readBytes(byteBuffer, capacity);

	if(receiveRSSI){
		Serial.readBytes(bytes, 0);
		rssiStrength = -256 + int(bytes[0]);
	}
}

void Lora::Package::ResetBytes(){
	delete [] byteBuffer;
	capacity = 0;
	headerOffset = 0;
	dataOffset = 0;
}

void Lora::Package::ResetBytes(uint8_t estimatedDataSize, uint8_t headerSize){
	ResetBytes();
	headerOffset = headerSize;
	capacity = headerSize + estimatedDataSize;
	byteBuffer = new byte[capacity];
}

void Lora::Package::SetHeader(unsigned int address, uint8_t channel){
	byte* bytes = new byte[3];
	bytes[0] = address >> 8;
	bytes[1] = address & 0xFF;
	bytes[2] = channel;
	WriteBytes(bytes, 3, 0);
	delete [] bytes;
}

void Lora::Package::WriteBytes(byte* bytes, uint8_t size, uint8_t offset){
	if (offset + size > 240) return;

	if(capacity < offset + size){
		
		byte* oldByteBuffer = byteBuffer;
		uint8_t oldCapacity = capacity;
		capacity = offset + size;

		//Enlarge Array and copy old entries
		byteBuffer = new byte[capacity];
		memcpy(byteBuffer, oldByteBuffer, combinedOffset());

		delete [] oldByteBuffer;
	}

	memcpy(&byteBuffer[offset], bytes, size);

	if(offset + size > combinedOffset()){
		dataOffset = offset + size - headerOffset;
	}
}

void Lora::Package::WriteBytes(byte* bytes, uint8_t size){
	WriteBytes(bytes, size, combinedOffset());
}

void Lora::Package::Send(){
	byteBuffer[headerOffset - 1] = combinedOffset();
	
	Serial.flush(); 
	while(millis() < nextAvailableSendTime){}; // Wait for the last data to be fully transmitted
	Serial.write(byteBuffer, combinedOffset());
	nextAvailableSendTime = millis() + (combinedOffset()) * 40;
}

byte* Lora::GetPackage(){
	return receivingPackage.byteBuffer;
}

unsigned int Lora::GetReceivingPackageSize(){
	return receivingPackage.combinedOffset();
}
unsigned int Lora::GetSendingPackageSize(){
	return sendingPackage.combinedOffset();
}

int Lora::GetRSSI(){
	return receivingPackage.rssiStrength;
}

int Lora::GetLastNoiseStrength(){
	return lastNoise;
}

int Lora::RequestSNR(){
	byte data[] = {0xC0, 0xC1, 0xC2, 0xC3, 0x00, 0x01};
	Serial.write(data, sizeof(data));

	//Wait for answer of Lora-Chip
	while(!Serial.available()){};
	
	byte byteBuffer[4];
	Serial.readBytes(byteBuffer, 4);

	//Save last Noise
	lastNoise = -256 + int(byteBuffer[3]);
	lastSNR = receivingPackage.rssiStrength - lastNoise;
	return lastSNR;
}
