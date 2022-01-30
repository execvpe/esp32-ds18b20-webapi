# ESP32 Temperature Sensor
ESP32 Web-API that can provide sensor readings from DS18B20 temperature sensors on the network


## Build

- Import as a new PlatformIO Project in VS Code
- Set up your WiFi credentials:
  - Create a new *encData.hpp* header using my project [Credentials Obfuscator](https://github.com/execvpe/credentials-obfuscator) and place it in the include directory
  - Change the WiFiHandler by adding your hardcoded credentials in the begin() function
- Compile and upload to an ESP32 using PlatformIO

## Behavior

The ESP32 is ready immediately after booting. You can read out debug information via the serial interface.

## Communicate with the API

Send an HTTP-GET request to the web server listening on default port 80.

e.g. ```GET SENSOR/TEMPERATURE/0/VALUE/C HTTP/1.1```

### *Sensor* requests:

```SENSOR/TEMPERATURE/<n>/VALUE/<C|F>``` returns a ```float``` formatted with ```%.2f```:
```
21.75
```

```SENSOR/TEMPERATURE/<n>/ELAPSED_TIME/MS``` returns an ```unsigned long```:
```
532
```

```SENSOR/TEMPERATURE/<n>/_ALL``` returns all (meta)data:
```
21.75 C
71.15 F
8108 MS
```

**All sensor requests may return ```INVALID READ: -127``` on failure.**

### *Actor/Buzzer* requests (beta!):


```ACTOR/BUZZER/<n>/OFF``` and ```ACTOR/BUZZER/<n>/ON[/<value>]``` (unit: milliseconds) return
```
OK. Buzzer <n>: OFF|ON|BEEP/SIGNAL
```
