# ESP32 Temperature Sensor
ESP32 Web-API that can provide sensor readings from DS18B20 temperature sensors on the network

## Build

- Import as a new PlatformIO Project in VS Code
- Set up your WiFi credentials:
  - Create a new *obfuscatedData.hpp* header using my project [Credentials Obfuscating Tool](https://github.com/execvpe/credentials-obfuscating-tool) and place it in the *lib/deobfuscate/src/* directory
  - Alternatively: Change the WiFiHandler by adding your hardcoded credentials in the begin() function (C/C++ knowledge required)
- Compile and upload to an ESP32 using PlatformIO

## Default behavior

The ESP32 is ready immediately after booting. You can read out debug information via the serial interface.

## Communicate with the API

Send a `GET` (HTTP/1.0 or HTTP/1.1) request to the web server listening on default port `80`.

e.g. `GET SENSOR/TEMPERATURE/0/VALUE/C HTTP/1.1`

### *Sensor* requests:

`SENSOR/TEMPERATURE/<n>/VALUE/<C|F>` returns a `float` (format: `%.2f`):

```
21.75
```

**Sensor requests may return ```INVALID READ: -127``` on failure.**

### *Actor/Buzzer* requests:

`ACTOR/BUZZER/<n>/OFF` and `ACTOR/BUZZER/<n>/ON[/<value>]` (unit: milliseconds) return a `string`:

```
OK. Buzzer <n>: OFF|ON|BEEP/SIGNAL
```

## Licensing

Any code not written by me, I release under the license under which it was originally released. If the license of any third-party code is not explicitly stated in the file *[CONTRIBUTORS.md](CONTRIBUTORS.md)* or with the third-party code itself, please contact the original author for more information on the license used.

My code is released under the global project license in *[LICENSE](LICENSE)*.
