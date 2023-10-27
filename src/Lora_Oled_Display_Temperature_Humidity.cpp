#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_Sensor.h" // Manually add libray for PIO
#include "Arduino_SensorKit.h"
//#include "Adafruit_AHTX0.h"
#include "SparkFun_SCD30_Arduino_Library.h"
#include "CustomLoraNetwork.h"

#define M0_PIN 2
#define M1_PIN 3 

SCD30 airSensor;
/* Adafruit_AHTX0 aht;
sensors_event_t humidity_event, temp_event; */
CustomLoraNetwork loraNetwork(M0_PIN,M1_PIN);
float pressure, temperature = 1, humidity = 1;
unsigned int co2ppm = 1;

String rcvData;
unsigned long lastTimeSensor = 0;
unsigned long lastTimeOLED = 0;
bool stateChange = false;

int maxSamples = 4;
int samplesCounter = 0;

extern char  *__brkval;
int freeRAM() {
  return 2048 -  (int) __brkval;
}

float VC = 5.0;
int RL = 1500;

void setup() {
	Serial.begin(9600);
	Wire.begin();
	Wire.setWireTimeout(25000,true);
	Oled.begin();
	Oled.setFlipMode(true);
	Pressure.begin();
	airSensor.begin();
	airSensor.useStaleData(true);
	airSensor.setAltitudeCompensation(200);
	//airSensor.setTemperatureOffset(0);
	//aht.begin();
	
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

		if(airSensor.dataAvailable()){
			co2ppm = airSensor.getCO2();
			temperature = airSensor.getTemperature();
			humidity = airSensor.getHumidity();
		}

		pressure = Pressure.readPressure();

		Oled.clearDisplay();

		Oled.setFont(u8x8_font_chroma48medium8_r);
		Oled.setCursor(0, 0);
		Oled.print(F("Ram: "));
		Oled.print(freeRAM());
			
		Oled.setCursor(0, 1);
		Oled.print(F("CO2: "));
		Oled.print(co2ppm); 
		Oled.print(F(" ppm"));
		Oled.setCursor(0, 2);
		Oled.print(F("Temp: "));
		Oled.print(temperature);
		Oled.print(F(" C"));
		Oled.setCursor(0, 3);
		Oled.print(F("Hum: "));
		Oled.print(humidity);
		Oled.print(F(" %"));
		Oled.setCursor(0, 4);
		Oled.print(F("Pres: "));
		Oled.print(pressure/100000.0);
		Oled.print(F(" Bar"));
		Oled.setCursor(0, 5);

		/*Oled.setCursor(0, 1);
		Oled.print(F("MID: "));
		Oled.print(loraNetwork.lastMainUnitID);
		Oled.setCursor(0, 2);
		Oled.print(F("ID: "));
		Oled.print(loraNetwork.ID);
		Oled.setCursor(0, 3);
		Oled.print(F("Add: "));
		Oled.print(loraNetwork.nodeAddress); 
		Oled.setCursor(0, 4);
		Oled.print(F("PID: "));
		Oled.print(loraNetwork.parentID);
		Oled.setCursor(0, 5);
		Oled.print(F("PAdd: "));
		Oled.print(loraNetwork.parentAddress);
		Oled.setCursor(0, 6);
		Oled.print(F("CRC: "));
		Oled.print(loraNetwork.setupChecksum);*/

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

	 if(loraNetwork.CustomDataRequested){//curTime - lastTimeSensor >= 1000){
		Oled.setCursor(0,7);
		Oled.print(F("GO DATA!"));
		lastTimeSensor = curTime;
		co2ppm = airSensor.getCO2();
		temperature = airSensor.getTemperature();
		humidity = airSensor.getHumidity();
		pressure = Pressure.readPressure();
		loraNetwork.CustomDataAvailable = true;
	}
}
