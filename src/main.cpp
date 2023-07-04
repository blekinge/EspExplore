// This sets up Logging. 4 i DEBUG, 3 is INFO
// #define CORE_DEBUG_LEVEL 4
#ifdef ARDUINO_ARCH_ESP32

#include "esp32-hal-log.h"

#endif

#include <Arduino.h>

#include "Provisioner.h"
#include <ESPmDNS.h>
#include "Ota.h"
#include <WiFi.h>
#include <Preferences.h>
#include <TelnetSpy.h>
#include <ArduinoJson.h>


static const String prefNamespace = "EspExplore";


TelnetSpy SerialAndTelnet;
// #define SER  Serial
#define SER SerialAndTelnet

bool startMDNS(String host) {
    const char *hostname = host.c_str();
    WiFiClass::setHostname(hostname);
    if (MDNS.begin(host)) {
        log_i("mDNS responder started. Open http://%s.local/ in your browser\n", hostname);

        MDNS.enableArduino();
        MDNS.enableWorkstation();

        MDNS.addService("telnet", "tcp", 23);

        const IPAddress &address = MDNS.queryHost(host);
        log_i("This host should have ip %s", address.toString().c_str());

        return true;
    }

    return false;
}

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

extern const char config_default[] asm("_binary_src_defaults_json_start");

void setup() {

    setupSerial();

    Preferences prefs;
    prefs.begin(prefNamespace.c_str(), false);

    String configString = prefs.getString("config");
    if (configString == nullptr) {
        //TODO merge existing settings with defaults, as the defaults might have been updated?
        prefs.putString("config", config_default);
        configString = prefs.getString("config");
    }

    DynamicJsonDocument config(1024);
    deserializeJson(config, configString);

    const char *sensor = config["sensor"];
    log_i("sensor %s", sensor);

    config["sensor"].set("aps");
    String jsonOut = "";
    serializeJson(config, jsonOut);
    log_d("output json is %s", jsonOut.c_str());
    prefs.putString("config", jsonOut);

    String softAPname = config["provision"]["softAPname"];
    String pop = config["provision"]["pop"];
    provisionWithSoftAP(softAPname, pop);

    String otaUrl = config["ota"]["url"];
    // otaUpdate(otaUrl);

    String hostname = prefs.getString("hostname", String(WiFiClass::getHostname()));
    startMDNS(hostname);

    prefs.end();
}

void loop() {
    SerialAndTelnet.handle();

    log_i("IP address: %s", WiFi.localIP().toString().c_str());

    // otaUpdate(otaUrl);
    delay(1000);
}
