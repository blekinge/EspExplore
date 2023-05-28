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

// WebServer httpServer(80);
// HTTPUpdateServer httpUpdater;
Preferences prefs;
// TCP server at port 80 will respond to HTTP requests
WiFiServer httpServer(80);

void store();
void retrieve();

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

  MDNS.begin(host);
  if (MDNS.begin(host))
  {
    log_i("mDNS responder started");
  }


  // httpUpdater.setup(&httpServer);
  httpServer.begin();
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  log_i("HTTPUpdateServer ready! Open http://%s.local/ in your browser\n", host);

  // store();

  // retrieve();
}

void loop()
{
  // put your main code here, to run repeatedly:
  // Serial.println("Looping");

   // Check if a client has connected
    WiFiClient client = httpServer.available();
    if (!client) {
        return;
    }
    
    log_i("New client");

    // Wait for data from client to become available
    while(client.connected() && !client.available()){
        delay(1);
    }

    // Read the first line of HTTP request
    String req = client.readStringUntil('\r');

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    if (addr_start == -1 || addr_end == -1) {
        log_w("Invalid request: %s", req);    
        return;
    }
    req = req.substring(addr_start + 1, addr_end);
    log_i("Request: %s", req);
    
    String s;
    if (req == "/")
    {
        IPAddress ip = WiFi.localIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP32 at ";
        s += ipStr;
        s += "</html>\r\n\r\n";
        log_i("Sending 200");
    }
    else
    {
        s = "HTTP/1.1 404 Not Found\r\n\r\n";
        log_i("Sending 404");
    }
    client.print(s);

    client.stop();
    Serial.println("Done with client");
}

void store()
{
  prefs.begin("namespace");

  size_t sizeUrls = sizeof(urls);
  size_t numBytes = prefs.putBytes("sides", urls, sizeof(urls));

  log_d("bytes stored: %u from orig %u", numBytes, sizeUrls);
  prefs.end();
}

void retrieve()
{
  prefs.begin("namespace");

  int i = 0;

  size_t schLen = prefs.getBytesLength("sides");
  log_d("%u", i++);
  char buffer[schLen]; // prepare a buffer for the data
  log_d("%u", i++);
  prefs.getBytes("sides", buffer, schLen);
  log_d("%u", i++);
  if (schLen % sizeof(endpoint_t))
  { // simple check that data fits
    log_w("Data is not correct size!");
    return;
  }

  log_d("%u", i++);
  endpoint_t *endpoints = (endpoint_t *)buffer; // cast the bytes into a struct ptr
  log_d("%u", i++);
  log_i("%s", endpoints[0].httpMethod);
  prefs.end();
}