# Arduino_Sensorknoten
## Code Compilation
Code is Compiled with PlatformIO addon for VSCode.
All compile instructions are inside the platformio.ini.

If you are instead only using the default Arduino IDE, you need to install following packets:
- hideakitai/ArxTypeTraits@^0.2.3
- sparkfun/SparkFun SCD30 Arduino Library@^1.0.20
- bakercp/CRC32@^2.0.0

Also a manual install of these is needed inside the correct folder names (in front of the equal sign =):
- Grove - Barometer Sensor BMP280=https://github.com/Seeed-Studio/Grove_BMP280.git#8566d467418d20600119277a64e85c6a4fafe328
- Arduino_Sensorkit=https://github.com/arduino-libraries/Arduino_SensorKit.git#v1.0.8

## Description

Official Extract of my german Master Thesis with the title "Development of a wireless sensor network for monitoring of deep geological
repositories in the operational phase":
> In den LoRa-Bibliotheken Lora und LoraSetup befinden sich die möglichen Einstellungen des LoRa-Moduls.
> Zudem wird dort das Auslesen und Senden von Bytes, maßgefertigt für das Modul SX1262, gesteuert.
> Genauere Infos zu den möglichen Einstellungen befinden sich dazu in der [Waveshare-Wiki](https://www.waveshare.com/wiki/SX1262_868M_LoRa_HAT).
> Dort sind alle wichtigen für das LoRa-Modul spezifischen Verfahren und Befehle hinterlegt.
> Mit der Netzwerk-Bibliothek CustomLoraNetwork werden alle Synchronisierungsverfahren spezifisch für das geplante Überwachungsnetzwerk festgelegt und auch die Konventionen für die einzelnen Variablen geregelt.

Translation:
The LoRa-Libraries Lora.cpp and LoraSetup.cpp contain the possible options for the LoRa-Module.
They handle the Reading and Sending of Bytes, customized for the module SX1262.
More Information are located in the [Waveshare-Wiki](https://www.waveshare.com/wiki/SX1262_868M_LoRa_HAT)
All important specific procedures and commands are mentioned there, one way or another.
The network-library CustomLoraNetwork.cpp handles all synchronization specific to the planned Monitoring network and the conventions for single variables.
