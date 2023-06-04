// This sets up Logging. 4 i DEBUG, 3 is INFO
// #define CORE_DEBUG_LEVEL 4
#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif

#include <Arduino.h>

#include "Provisioner.h"

#include <ESPmDNS.h>

#include "config.h"
#include "Ota.h"

#include <WiFi.h>

// static const String otaUrl = "https://gameon.askov.net:4443/.pio/build/az-delivery-devkit-v4/firmware.bin"; // state url of your firmware image
static const String otaUrl = "https://gameon.askov.net:8443/OTA/firmware/firmware.bin"; // state url of your firmware image

bool startMDNS(String host="esp32")
{
  if (MDNS.begin(host))
  {
    log_i("mDNS responder started. Open http://%s.local/ in your browser\n", host);
    return true;
  }
  return false;
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  String softAPname = "Prov123";
  String pop = "abcd1234";
  provisionWithSoftAP(softAPname, pop);
  startMDNS();
  otaUpdate(otaUrl);

}

void loop()
{
  log_i("IP address: %s", WiFi.localIP().toString());
  otaUpdate(otaUrl);

  delay(10000);
}
