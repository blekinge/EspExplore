// This sets up Logging. 4 i DEBUG, 3 is INFO
// #define CORE_DEBUG_LEVEL 4
#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif

#include <Arduino.h>

#include "Provisioner.h"

#include <ESPmDNS.h>

#include "config.h"

#include <WiFi.h>

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

  // startMDNS();
  String softAPname = "Prov123";
  String pop = "abcd1234";
  provisionWithSoftAP(softAPname, pop);
}

void loop()
{
  log_i("IP address: %s", WiFi.localIP().toString());
  delay(1000);

}
