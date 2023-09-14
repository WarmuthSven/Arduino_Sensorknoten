#ifndef LORASETUP_H
#define LORASETUP_H

#include "Arduino.h"

enum LORA_BAUDRATE : uint8_t{
	BAUDRATE_1200 = 0b000,
	BAUDRATE_2400 = 0b001,
	BAUDRATE_4800 = 0b010,
	BAUDRATE_9600 = 0b011,
	BAUDRATE_19200 = 0b100,
	BAUDRATE_38400 = 0b101,
	BAUDRATE_57600 = 0b110,
	BAUDRATE_115200 = 0b111
};

enum LORA_PARITYBIT : uint8_t{
	PARITY_8N1 = 0b00,
	PARITY_8O1 = 0b01,
	PARITY_8E1 = 0b10,
};

enum LORA_AIRSPEED : uint8_t{
	AIRSPEED_300 = 0b000,
	AIRSPEED_1200 = 0b001,
	AIRSPEED_2400 = 0b010,
	AIRSPEED_4800 = 0b011,
	AIRSPEED_9600 = 0b100,
	AIRSPEED_19200 = 0b101,
	AIRSPEED_38400 = 0b110,
	AIRSPEED_62500 = 0b111
};

enum LORA_PACKETSIZE : uint8_t{
	SIZE_240 = 0b00,
	SIZE_128 = 0b01,
	SIZE_64 = 0b10,
	SIZE_32 = 0b11
};

enum LORA_TRANSMISSIONPOWER : uint8_t{
	POWER_22 = 0b00,
	POWER_17 = 0b01,
	POWER_13 = 0b10,
	POWER_10 = 0b11
};

enum LORA_TRANSMISSIONMODE : uint8_t{
	TRANSMISSION_TRANSPARENT = 0,
	TRANSMISSION_FIXEDPOINT = 1
};

enum LORA_WORMODE : uint8_t{
	WOR_TRANSMIT = 0,
	WOR_RECEIVER = 1
};

enum LORA_WORPERIOD : uint8_t{
	WOR_500 = 0b000,
	WOR_1000 = 0b001,
	WOR_1500 = 0b010,
	WOR_2000 = 0b011,
	WOR_2500 = 0b100,
	WOR_3000 = 0b101,
	WOR_3500 = 0b110,
	WOR_4000 = 0b111
};



//Exact Address Description @https://www.waveshare.com/wiki/LoRa-HAT-Reg
class LoraSetup{
	public:
		//Set Defaults
		LoraSetup();

		//00H + 01H
		uint16_t address; // 2^16 - 1 = 65535 0xFFFF Broadcast Address

		//02H
		uint8_t netID;

		//03H
		LORA_BAUDRATE baudRate;
		LORA_PARITYBIT parityBit;
		LORA_AIRSPEED airSpeed;

		//04H
		LORA_PACKETSIZE packetLength;
		bool enableAmbientNoise;
		LORA_TRANSMISSIONPOWER transmitPower;

		//05H
		uint8_t channel; //0-83 possible Channels

		//06H
		bool enableRSSI;
		LORA_TRANSMISSIONMODE transmissionMethod;
		bool enableRelay;
		bool enableLBT;
		LORA_WORMODE WORMode;
		LORA_WORPERIOD WORPeriod;

		//07H + 08H
		uint16_t encryptionKey;

		byte* ReturnByteArray();
		void SetAddress(unsigned int address);
		void SetNetID(uint8_t id);
		void SetBaudRate(LORA_BAUDRATE baudRate);
		void SetParityBit(LORA_PARITYBIT parityBit);
		void SetAirSpeed(LORA_AIRSPEED airSpeed);
		void SetPacketSize(LORA_PACKETSIZE packetSize);
		void EnableAmbientNoiseMeasurement(bool enable);
		void SetTransmitPower(LORA_TRANSMISSIONPOWER transmitPower);
		void SetChannel(uint8_t channel);
		void EnableRSSIMeasurement(bool enable);
		void SetTransmissionMethod(LORA_TRANSMISSIONMODE transmissionMode);
		void EnableRelayMode(bool enable);
		void EnableLBT(bool enable);
		void SetWORMode(LORA_WORMODE worMode);
		void SetWORPeriod(LORA_WORPERIOD worPeriod);
		void SetEncryptionKey(uint16_t key);

	private:
		static const uint8_t baudRateShift = 5;
		static const uint8_t baudRateDeleteMask = 0b00011111;
		static const uint8_t parityBitShift = 3;
		static const uint8_t parityBitDeleteMask = 0b11100111;
		static const uint8_t airSpeedDeleteMask = 0b11111000;
		static const uint8_t packetLengthShift = 6;
		static const uint8_t packetLengthDeleteMask = 0b00111111;
		static const uint8_t enableAmbientNoiseShift = 5;
		static const uint8_t enableAmbientNoiseDeleteMask = 0b11011111;
		static const uint8_t transmitPowerDeleteMask = 0b11111100;
		static const uint8_t enableRSSIShift = 7;
		static const uint8_t enableRSSIDeleteMask = 0b01111111;
		static const uint8_t transmissionMethodShift = 6;
		static const uint8_t transmissionMethodDeleteMask = 0b10111111;
		static const uint8_t enableRelayShift = 5;
		static const uint8_t enableRelayDeleteMask = 0b11011111;
		static const uint8_t enableLBTShift = 4;
		static const uint8_t enableLBTDeleteMask = 0b11101111;
		static const uint8_t WORModeShift = 3;
		static const uint8_t WORModeDeleteMask = 0b11110111;
		static const uint8_t WORPeriodDeleteMask = 0b11111000;

		byte RegisterSetup[12];
};

#endif
