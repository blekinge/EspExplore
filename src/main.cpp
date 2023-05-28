// This sets up Logging. 4 i DEBUG, 3 is INFO
#define CORE_DEBUG_LEVEL 4
#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif

#include <Arduino.h>

#include "config.h"
#include <Preferences.h>

#include <HTTPUpdateServer.h>
#include <ESPmDNS.h>

WebServer httpServer(80);
// HTTPUpdateServer httpUpdater;
Preferences prefs;
// TCP server at port 80 will respond to HTTP requests
// WiFiServer httpServer(80);
const int led = 13;


void handleNotFound();
void handleRoot();

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println();

  /*
    To upload through terminal you can use: curl -F "image=@firmware.bin" esp32-webupdate.local/update
  */
  log_i("Booting Sketch...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(500);
    WiFi.begin(ssid, password);
    log_w("WiFi failed, retrying.");
  }
  log_i("Connected to %s", ssid);
  log_i("IP address: %s", WiFi.localIP().toString());

  if (MDNS.begin(host))
  {
    log_i("mDNS responder started");
    log_i("HTTPUpdateServer ready! Open http://%s.local/ in your browser\n", host);
  }
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  

  httpServer.on("/", handleRoot);

  httpServer.on("/inline", []()
                { httpServer.send(200, "text/plain", "this works as well"); });

  httpServer.onNotFound(handleNotFound);

  httpServer.begin();
  Serial.println("HTTP server started");

  // store();

  // retrieve();
}

void handleRoot()
{
  digitalWrite(led, 1);
  httpServer.send(200, "text/plain", "hello from esp32!");
  digitalWrite(led, 0);
}

void handleNotFound()
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";
  for (uint8_t i = 0; i < httpServer.args(); i++)
  {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }
  httpServer.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void loop()
{
  httpServer.handleClient();
  delay(2); // allow the cpu to switch to other tasks
}
