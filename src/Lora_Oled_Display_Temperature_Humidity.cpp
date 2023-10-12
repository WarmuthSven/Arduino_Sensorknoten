#include "Arduino.h"
#include "Arduino_SensorKit.h"
#include "Adafruit_AHTX0.h"
#include "SparkFun_SCD30_Arduino_Library.h"
#include "CustomLoraNetwork.h"

#define M0_PIN 2
#define M1_PIN 3 

SCD30 airSensor;
/* Adafruit_AHTX0 aht;
sensors_event_t humidity_event, temp_event; */
CustomLoraNetwork loraNetwork(M0_PIN,M1_PIN);
float pressure, temperature, humidity;
uint16_t co2ppm;

String rcvData;
unsigned long lastTimeSensor = 0;
unsigned long lastTimeOLED = 0;
bool stateChange = false;

int maxSamples = 3;
int samplesCounter = 0;

extern char  *__brkval;
int freeRAM() {
  return 2048 -  (int) __brkval;
}

float VC = 5.0;
int RL = 1500;

void setup() {
	Serial.begin(9600);
	Oled.begin();
	Pressure.begin();
	airSensor.begin();
	airSensor.setTemperatureOffset(0);
	//aht.begin();
	
	Oled.setFlipMode(true);


	//pinMode(A0, INPUT);

	loraNetwork.Init(maxSamples);
	loraNetwork.AddDataPointer(&pressure, "Luftdruck");
	loraNetwork.AddDataPointer(&co2ppm, "CO2");
	loraNetwork.AddDataPointer(&temperature, "Temperatur");
	loraNetwork.AddDataPointer(&humidity, "Feuchtigkeit");
}

short sentPackageSize = 0;
short receivedPackageSize = 0;
short packageEnum = 0;
short receivedRandom = 0;
int rssiStrength = 0;
int ambientNoise = 0;
int snrValue = 0;

int maxTests = 10;
int testCounter = 0;

void loop() {
	unsigned long curTime = millis();

	loraNetwork.Update();
	/*
	snrValue = lora.RequestSNR();
	ambientNoise = lora.GetLastNoiseStrength();
	*/
	if(curTime - lastTimeOLED >= 2000){		
		
		lastTimeOLED = curTime;

		/* packageEnum = loraNetwork.lastPackageID;
		rssiStrength = loraNetwork.GetLastRSSI();
 */
		Oled.clearDisplay();

		Oled.setFont(u8x8_font_chroma48medium8_r);
		Oled.setCursor(0, 0);
		Oled.print("Ram: " + String(freeRAM()));
		if(airSensor.dataAvailable()){
			airSensor.setAltitudeCompensation(Pressure.readAltitude());
			Oled.setCursor(0, 1);
			Oled.print("CO2: " + String(airSensor.getCO2()) + "ppm");
			Oled.setCursor(0, 2);
			Oled.print("Temp: " + String(airSensor.getTemperature()) + "°C");
			Oled.setCursor(0, 3);
			Oled.print("Hum: " + String(airSensor.getHumidity()) + "%");
		}
		
		Oled.setCursor(0, 4);
		Oled.print("Pres: " + String(Pressure.readPressure()/100000.0) + "Bar");
		Oled.setCursor(0, 5);
		Oled.print("Alt: " + String(Pressure.readAltitude()) + "m");

		/* Oled.setCursor(0, 6);
		aht.getEvent(&humidity_event, &temp_event);
		Oled.print("Temp2: " + String(temp_event.temperature) + "°C");
		Oled.setCursor(0, 7);
		Oled.print("Hum2: " + String(humidity_event.relative_humidity) + "%"); */
		/*
		Oled.setCursor(0, 1);
		Oled.print("rec "); 
		Oled.print(loraNetwork.recMessageCounter);
		Oled.print(", sen ");
		Oled.print(loraNetwork.sentMessageCounter);
		Oled.setCursor(0, 2);
		Oled.print("Address ");
		Oled.print(loraNetwork.nodeAddress);  */

		/*Oled.print(rssiStrength);
		Oled.print(" RSSI"); */

		/* float ADCval = analogRead(A0);
		float VRL = (5.0 / 1023) * ADCval;
		float RS = (VC / VRL - 1) * RL;
		Oled.print("RS = ");
		Oled.print(RS); */

		//String data = loraNetwork.GetData(samplesCounter);
		/*Oled.setCursor(0, 3);
		Oled.print("State: ");
		 Oled.print(loraNetwork.currentNetworkState);
		//Oled.print("ID: Rec/Sent");
		Oled.setCursor(0, 4);
		Oled.print("rec ");
		Oled.print(loraNetwork.receivedID);
		Oled.setCursor(0, 5);
		Oled.print("sen ");
		Oled.print(loraNetwork.ID); 
		Oled.setCursor(0, 6);
		Oled.print("Enum: ");
		Oled.print(packageEnum);
		Oled.setCursor(0, 7);
		Oled.print("Size: ");
		Oled.print(String(loraNetwork.GetLastReceivingPackageSize()));
		Oled.print("/");
		Oled.print(String(loraNetwork.GetLastSendingPackageSize())); */

		/*
		Oled.setCursor(0, 3);
		Oled.print("Real Storage:");
		Oled.setCursor(0, 4);
		Oled.print(data);

		String recData = loraNetwork.GetRecData(samplesCounter);
		Oled.setCursor(0, 6);
		Oled.print("Received:");
		Oled.setCursor(0, 7);
		Oled.print(recData); 
		
		Oled.setCursor(0, 6);
		Oled.print("Sen/Rec Size:");
		Oled.setCursor(0, 7);
		Oled.print(String(loraNetwork.GetLastSendingPackageSize()) + " / " + String(loraNetwork.GetLastReceivingPackageSize())); 
		*/

		samplesCounter++;
		samplesCounter%= maxSamples;

		/*Oled.setCursor(0, 3);
		Oled.print("Received Random:");
		Oled.setCursor(0, 4);
		Oled.print(receivedRandom);

		Oled.setCursor(0, 6);
		Oled.print("RSSI/Noise/SNR: ");
		Oled.setCursor(0, 7);
		Oled.print(rssiStrength);
		Oled.print(" / ");
		Oled.print(ambientNoise);
		Oled.print(" / ");
		Oled.print(snrValue);*/
		Oled.refreshDisplay();
	}

	 if(loraNetwork.CustomDataRequested && airSensor.dataAvailable()){//curTime - lastTimeSensor >= 1000){
		Oled.setCursor(0,5);
		Oled.print("GO DATA!");
		lastTimeSensor = curTime;
		co2ppm = airSensor.getCO2();
		temperature = airSensor.getTemperature();
		humidity = airSensor.getHumidity();
		pressure = Pressure.readPressure();
		loraNetwork.CustomDataAvailable = true;
	}
}
