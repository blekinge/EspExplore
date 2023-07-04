# Esp32 Template project

This project was made to serve as a template project, i.e. to contain the features and setups I would need in most other Esp32 projects

It is set up for the `az-delivery-devkit-v4` board, as these are the boards I have. 
The template is based on the [arduino-espressif32](https://github.com/espressif/arduino-esp32) framework.

Features include:
 * Wifi Provisioning
 * Over-The-Air updates (OTA)
 * Preferences, i.e. store user settings in the device
 * MicroDNS
 * Serial Interface over Telnet

## Provisioning

The provisining is based on <https://github.com/espressif/esp-idf/tree/v5.1/examples/provisioning>

Provisioning is handled by this lines in main.cpp/setup function
```c++
String softAPname = prefs.getString("provision_softAPname", "Prov123");
String pop = prefs.getString("provision_pop", "abcd1234");
provisionWithSoftAP(softAPname, pop);
```

You can set up the WIFI-provision in a number of ways

TODO provisioning with app

### Provising from the console

From <https://github.com/espressif/esp-idf> you can get the tool `esp_prov`

When the device boots up, it creates a new AP, named `Prov_123`
In this Wifi network, the device will have the IP 192.168.4.1 (TODO why this IP?)

Connect to this AP and run

```bash
/home/abr/Arduino/esp/esp-idf/tools/esp_prov/esp_prov.py \
    --transport 'softap' \
    --service_name '192.168.4.1' \
    --sec_ver 1 \
    --pop 'abcd1234' \
    --ssid 'YOUR_WIFI_SID' \
    --passphrase 'YOUR_WIFI_PASSWORD'
```

If `esp_prov.py` gives errors about python or modules, do

```bash
cd $HOME/Arduino/esp/esp-idf/
./install.sh
source ~/.espressif/python_env/idf5.0_py3.11_env/bin/activate
pip install bleak future protobuf=4.21 cryptography
```

## OTA
Over The Air updates are done with these lines
```c++
String otaUrl = prefs.getString("otaUrl", "https://gameon.askov.net:8443/OTA/firmware/firmware.bin");
otaUpdate(otaUrl);
```

### OTA server
First, you need a server serving the updates. As I am a Java programmer at heart, I have created a simple quarkus server, which can be found in `OTAserver/` 

Run the java (quarkus) server with `mvn quarkus:run`
It serves the firmware file from `.pio/build/az-delivery-devkit-v4/firmware.bin`

Note that a simple HTTP server will not cut it. The OTA client will send these additional headers
```
x-ESP32-AP-MAC=0C:B8:15:F8:AD:1D
x-ESP32-chip-size=4194304
x-ESP32-free-space=1966080
x-ESP32-mode=sketch
x-ESP32-sdk-version=v4.4.4
x-ESP32-sketch-md5=5c3d8fa3c3807f5431a87409ec479d57
x-ESP32-sketch-sha256=54323B12F98F9004415D7F7D13A87299D2799248B0858AFCDB62690157CFDAD6
x-ESP32-sketch-size=1743360
x-ESP32-STA-MAC=0C:B8:15:F8:AD:1C
```
The client will expect the server to reply with HTTP 304 (Not Modified) if the served firmware matches the incoming md5/sha1
If the server responds with a `firmware.bin` file, the client will perform an update, even if the version is not changed.
                                                 
### HTTPS
The OTA process require HTTPS. 
You can use any certificates you have, but if you need to, you can generate self-signed certificates with this command.

```bash
openssl req -new -x509 -keyout "$PWD/OTAserver/src/main/resources/ota.pem" -out ""$PWD/lib/Ota/ota.crt"" -days 365 -nodes  -subj "/CN=$(hostname)" && cp -f "$PWD/lib/Ota/ota.crt" "$PWD/OTAserver/src/main/resources/ota.crt" 
```
This generates the keyfile for the server and the `ota.crt` file for the device. This file is embedded on the device with
`board_build.embed_txtfiles = lib/Ota/ota.crt` in `platformio.ini`
and retrieved at runtime with
`extern const char rootCACertificate[] asm("_binary_lib_Ota_ota_crt_start");`
For more on this, see https://docs.platformio.org/en/latest/platforms/espressif32.html#embedding-binary-data

There are still spurious ssl timeout errors; probably related to <https://github.com/espressif/arduino-esp32/issues/7057>

### Auth
OTA server is protected with basic auth, `username@password`
TODO Currently this is not configurable, which is an obvious problem

## Preferences api
https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html

Preferences is an api for storing and retrieving simple values in Non-Volatile storage, i.e. permanent storage

It is initialized with these simple lines
```c++
Preferences prefs;
prefs.begin(prefNamespace.c_str(), false);
```
You can then retrieve values like 
```c++
String softAPname = prefs.getString("provision_softAPname", "Prov123");
```
The second value is the default value in case the key is not found

TODO preload preferences with default values centrally

## Telnet Serial
I would want to be able to debug via the serial interface, even when the device is not currently USB-connected.
The library https://github.com/yasheena/telnetspy allows me to do so

It is set up with
```c++

TelnetSpy SerialAndTelnet;
// The SER definition controls if we use Serial or SerialAndTelnet
// #define SER  Serial
#define SER SerialAndTelnet

void setupSerial() {

    SerialAndTelnet.setWelcomeMsg(F("Welcome to the TelnetSpy example\r\n\n"));

    SerialAndTelnet.setCallbackOnConnect([]() { SER.println(F("Telnet connection established.")); });

    SerialAndTelnet.setCallbackOnDisconnect([]() { SER.println(F("Telnet connection closed.")); });

    SerialAndTelnet.setFilter(char(1),
                              F("\r\nNVT command: AO\r\n"),
                              []() { SerialAndTelnet.disconnectClient(); });

    // Both needed to disable catching of system output
    // SER.setDebugOutput(false);
    // ets_install_putc1(ets_write_char_uart);

    SER.begin(115200);
    delay(100); // Wait for serial port
}
```

Afther the serial have been set up, you can now use it with commands like
`SER.println("hey");`, just as you would do with the standard `Serial` class.

You can now get the serial output of the device simply by connecting through Telnet
```bash
telnet esp32-f8ad1c.askov.net
```
### MDNS
? Why do I have this

### Logging
`Arduino-espressif32` provides a better logging framework than simply printing to the terminal. Import it by
```c++
#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif
```
You can now log like
`log_i("IP address: %s", WiFi.localIP().toString().c_str());` and this will be logged as
`[  3438][I][main.cpp:89] loop(): IP address: 192.168.2.180`
The format is `[Timestamp since bootup in MS][LogLevel][File:line] method(): message`


Perhaps?:

## <https://github.com/plerup/makeEspArduino/blob/master/README.md>
