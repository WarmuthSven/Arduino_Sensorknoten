#include "LoraSetup.h"

//Set Defaults
LoraSetup::LoraSetup(){
	RegisterSetup[0] = 0xC0; // Command to write Register
	RegisterSetup[1] = 0x00; // Begin Address
	RegisterSetup[2] = 0x09; // Length of Parameters to write (9 per default -> 00H - 08H)
	SetAddress(0x0000);
	SetNetID(0x00);
	SetBaudRate(BAUDRATE_9600);
	SetParityBit(PARITY_8N1);
	SetAirSpeed(AIRSPEED_2400);
	SetPacketSize(SIZE_240);
	EnableAmbientNoiseMeasurement(false);
	SetTransmitPower(POWER_22);
	SetChannel(0x00);
	EnableRSSIMeasurement(false);
	SetTransmissionMethod(TRANSMISSION_TRANSPARENT);
	EnableRelayMode(false);
	EnableLBT(false);
	SetWORMode(WOR_TRANSMIT);
	SetWORPeriod(WOR_500);
	SetEncryptionKey(0x0000);
}

byte* LoraSetup::ReturnByteArray(){
	return RegisterSetup;
}

//00H + 01H // 2^16 - 1 = 65535 0xFFFF Broadcast Address
void LoraSetup::SetAddress(unsigned int address){
	this->address = address;
	RegisterSetup[3] = address >> 8;
	RegisterSetup[4] = address & 0xFF;
}

//02H
void LoraSetup::SetNetID(uint8_t id){
	this->netID = id;
	RegisterSetup[5] = netID; 
}

//03H
void LoraSetup::SetBaudRate(LORA_BAUDRATE baudRate){
	this->baudRate = baudRate;
	RegisterSetup[6] = (RegisterSetup[6] & baudRateDeleteMask) | (baudRate << baudRateShift);
}

//03H
void LoraSetup::SetParityBit(LORA_PARITYBIT parityBit){
	this->parityBit = parityBit;
	RegisterSetup[6] = (RegisterSetup[6] & parityBitDeleteMask) | (parityBit << parityBitShift);
}

//03H
void LoraSetup::SetAirSpeed(LORA_AIRSPEED airSpeed){
	this->airSpeed = airSpeed;
	RegisterSetup[6] = (RegisterSetup[6] & airSpeedDeleteMask) | airSpeed;
}

//04H
void LoraSetup::SetPacketSize(LORA_PACKETSIZE packetSize){
	this->packetLength = packetSize;
	RegisterSetup[7] = (RegisterSetup[7] & packetLengthDeleteMask) | (packetLength << packetLengthShift);
}

//04H
void LoraSetup::EnableAmbientNoiseMeasurement(bool enable){
	this->enableAmbientNoise = enable;
	RegisterSetup[7] = (RegisterSetup[7] & enableAmbientNoiseDeleteMask) | (enableAmbientNoise << enableAmbientNoiseShift);
}

//04H
void LoraSetup::SetTransmitPower(LORA_TRANSMISSIONPOWER transmitPower){
	this->transmitPower = transmitPower;
	RegisterSetup[7] = (RegisterSetup[7] & transmitPowerDeleteMask) | transmitPower;
}

//05H //0-83 possible Channels
void LoraSetup::SetChannel(uint8_t channel){
	if(channel > 83){
		channel = 83;
	}
	this->channel = channel;
	RegisterSetup[8] = channel;
}

//06H
void LoraSetup::EnableRSSIMeasurement(bool enable){
	this->enableRSSI = enable;
	RegisterSetup[9] = (RegisterSetup[9] & enableRSSIDeleteMask) | (enableRSSI << enableRSSIShift);
}

//06H
void LoraSetup::SetTransmissionMethod(LORA_TRANSMISSIONMODE transmissionMode){
	this-> transmissionMethod = transmissionMode;
	RegisterSetup[9] = (RegisterSetup[9] & transmissionMethodDeleteMask) | (transmissionMethod << transmissionMethodShift);
}

//06H
void LoraSetup::EnableRelayMode(bool enable){
	this->enableRelay = enable;
	RegisterSetup[9] = (RegisterSetup[9] & enableRelayDeleteMask) | (enableRelay << enableRelayShift);
}

//06H
void LoraSetup::EnableLBT(bool enable){
	this->enableLBT = enable;
	RegisterSetup[9] = (RegisterSetup[9] & enableLBTDeleteMask) | (enableLBT << enableLBTShift);
}

//06H
void LoraSetup::SetWORMode(LORA_WORMODE worMode){
	this->WORMode = worMode;
	RegisterSetup[9] = (RegisterSetup[9] & WORModeDeleteMask) | (WORMode << WORModeShift);
}

//06H
void LoraSetup::SetWORPeriod(LORA_WORPERIOD worPeriod){
	this->WORPeriod = worPeriod;
	RegisterSetup[9] = (RegisterSetup[9] & WORPeriodDeleteMask) | WORPeriod;
}

//07H + 08H
void LoraSetup::SetEncryptionKey(uint16_t key){
	this->encryptionKey = key;
	RegisterSetup[10] = key >> 8;
	RegisterSetup[11] = key & 0xFF;
}
