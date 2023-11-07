#ifndef LORA_H
#define LORA_H

#include "Arduino.h"
#include "Datatypes.h"
#include "LoraSetup.h"

class Lora{
	class Package{
		private:
			uint8_t headerOffset = 0;
			uint8_t dataOffset = 0;
			uint8_t capacity = 0;
		public:
			int rssiStrength = 0;
			unsigned long nextAvailableSendTime;
			void ReadBytes(bool receiveRSSI);
			void WriteBytes(byte* bytes, uint8_t size);
			void WriteBytes(byte* bytes, uint8_t size, uint8_t offset);
			void Send();
			void SetHeader(unsigned int address, uint8_t channel);
			void ResetBytes();
			void ResetBytes(uint8_t estimatedDataSize, uint8_t headerSize);
			byte* byteBuffer = nullptr;
			uint8_t combinedOffset();
	};

	private:
		byte M0;
		byte M1;
		Package receivingPackage;
		Package sendingPackage;

		bool worMode;
		bool waitForNoiseRead;
		int lastSNR;
		int lastNoise;
		bool ReadyToSend;
		bool RegistrationState;

	public:
		//Setup Pins and call init()
		Lora(byte M0, byte M1);

		void Init(bool worMode);
		void Update();

		void ApplySetup();
		void StartNewSendPackage(uint8_t estimatedDataSize = 0);
		void StartNewSendPackage(unsigned int address, uint8_t channel, uint8_t estimatedDataSize = 0);
		void AddToSendPackage(byte* bytes, int size);
		void SendPackage();

		bool HasPackage();

		byte* GetPackage();
		unsigned int GetReceivingPackageSize();
		unsigned int GetSendingPackageSize();

		int GetRSSI();
		int GetLastNoiseStrength();
		int RequestSNR();

		LoraSetup setup;
};

#endif
