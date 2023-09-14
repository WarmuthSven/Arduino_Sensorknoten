#ifndef LORA_H
#define LORA_H

#include "Arduino.h"
#include "Datatypes.h"
#include "LoraSetup.h"

class Lora{
	class Package{
		private:
		public:
			byte byteBuffer[243];
			int rssiStrength = 0;
			int headerOffset;
			unsigned int packageSize = 0;
			unsigned long nextAvailableSendTime;
			void ReadBytes(bool receiveRSSI);
			void WriteBytes(byte* bytes, int size);
			void Send();
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
		void StartNewSendPackage();
		void StartNewSendPackage(unsigned int address, uint8_t channel);
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
