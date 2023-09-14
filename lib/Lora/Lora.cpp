#include "Lora.h"

Lora::Lora(byte M0, byte M1){
	this->M0 = M0;
	this->M1 = M1;

	receivingPackage.headerOffset = 1;
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

void Lora::StartNewSendPackage(){
	sendingPackage.packageSize = 0;
	sendingPackage.headerOffset = sizeof(byte); //Place for messageSize(1)
}

void Lora::StartNewSendPackage(unsigned int address, uint8_t channel){
	sendingPackage.packageSize = 0;
	sendingPackage.headerOffset = 0;

	if(channel > 83){
		channel = 83;
	}

	byte* bytes = new byte[3];
	bytes[0] = address >> 8;
	bytes[1] = address & 0xFF;
	bytes[2] = channel;
	sendingPackage.WriteBytes(bytes, 3);
	delete [] bytes;

	sendingPackage.packageSize = 0;
	sendingPackage.headerOffset = 4 * sizeof(byte); //Place for address (2), channel (1), messageSize(1)
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


void Lora::Package::ReadBytes(bool receiveRSSI){
	Serial.readBytes(byteBuffer, headerOffset);
	packageSize = DatatypeConverter::GetDataValue<byte>(byteBuffer);

	if(receiveRSSI){
		packageSize++;
	}

	Serial.readBytes(byteBuffer, packageSize);

	if(receiveRSSI){
		packageSize--; //decrease for correct overall package Size
		rssiStrength = -256 + int(byteBuffer[packageSize]);
	}
}

void Lora::Package::WriteBytes(byte* bytes, int size){
	if (packageSize + size > 240 - 1) return;

	memcpy(&byteBuffer[headerOffset + packageSize], bytes, size);
	packageSize += size;
}

void Lora::Package::Send(){
	memcpy(&byteBuffer[headerOffset - 1], &packageSize, 1);
	
	Serial.flush(); 
	while(millis() < nextAvailableSendTime){}; // Wait for the last data to be fully transmitted
	Serial.write(byteBuffer, headerOffset + packageSize);
	nextAvailableSendTime = millis() + (headerOffset + packageSize) * 40;
}

byte* Lora::GetPackage(){
	return receivingPackage.byteBuffer;
}

unsigned int Lora::GetReceivingPackageSize(){
	return receivingPackage.packageSize;
}
unsigned int Lora::GetSendingPackageSize(){
	return sendingPackage.packageSize;
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
