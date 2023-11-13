#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_Sensor.h" // Manually add libray for PIO
#include "Arduino_SensorKit.h"
#include "SparkFun_SCD30_Arduino_Library.h"
#include "CustomLoraNetwork.h"

#define M0_PIN 2
#define M1_PIN 3 

uint8_t methanPin = A3;
uint8_t coPin = A0;

SCD30 airSensor;
CustomLoraNetwork loraNetwork(M0_PIN,M1_PIN);

float pressure = 0, temperature = 0, humidity = 0;
unsigned int co2ppm = 0, coAmount = 0, methanAmount = 0;

unsigned long lastTimeOLED = 0;

extern char  *__brkval;
int freeRAM() {
  return 2048 -  (int) __brkval;
}

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
	
	pinMode(methanPin, INPUT);
	pinMode(coPin, INPUT);

	loraNetwork.Init(6);
	loraNetwork.AddDataPointer(&pressure, "Luftdruck");
	loraNetwork.AddDataPointer(&co2ppm, "CO2");
	loraNetwork.AddDataPointer(&temperature, "Temperatur");
	loraNetwork.AddDataPointer(&humidity, "Feuchtigkeit");
	loraNetwork.AddDataPointer(&methanAmount, "Methan");
	loraNetwork.AddDataPointer(&coAmount, "CO");
}

void loop() {
	unsigned long curTime = millis();

	loraNetwork.Update();

	if(curTime - lastTimeOLED >= 2000){		
		
		lastTimeOLED = curTime;

		if(airSensor.dataAvailable()){
			co2ppm = airSensor.getCO2();
			humidity = airSensor.getHumidity();
		}

		pressure = Pressure.readPressure();
		temperature = Pressure.readTemperature();

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
		Oled.print(F("Methan: "));
		float gas = analogRead(methanPin);
		Oled.print(gas * 100 / 1023.0);
		Oled.print(F(" %"));

		Oled.setCursor(0, 6);
		Oled.print(F("CO: "));
		gas = analogRead(coPin);
		Oled.print(gas * 100 / 1023.0);
		Oled.print(F(" %"));

		Oled.refreshDisplay();
	}

	 if(loraNetwork.NeedCustomDataUpdate()){
		Oled.setCursor(0,7);
		Oled.print(F("GO DATA!"));

		co2ppm = airSensor.getCO2();
		humidity = airSensor.getHumidity();
		temperature = Pressure.readTemperature();
		pressure = Pressure.readPressure();
		methanAmount = analogRead(methanPin);
		coAmount = analogRead(coPin);

		loraNetwork.ConfirmCustomDataUpdate();
	}
}
