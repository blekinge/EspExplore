#include <Arduino.h>
#include <LittleFS.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include <ESPmDNS.h>
#include <SD_MMC.h>
#include <FS.h>

#include <configAssist.h>  // Config assist class

WebServer server(80);


//Define application name
#define APP_NAME "ConfigAssistDemo"

// Default application config dictionary
// Modify the file with the params for you application
// Then you can use then then by val = config[name];
extern const char appConfigDict_json[] asm("_binary_src_appConfigDict_json_start");

ConfigAssist conf;                    // Config class
String hostName;                      // Default Host name
unsigned long pingMillis = millis();  // Ping 

// *********** Helper funcions ************
void debugMemory(const char* caller) {
    LOG_DBG("%s > Free: heap %u, block: %u, pSRAM %u\n", caller, ESP.getFreeHeap(), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), ESP.getFreePsram());

}
// List storage file system
void ListDir(const char * dirname) {
    LOG_INF("Listing directory: %s\n", dirname);
    // ist details of files on file system
    File root = STORAGE.open(dirname,"r");
    File file = root.openNextFile();
    while (file) {
        LOG_DBG("File: %s, size: %u B\n", file.path(), file.size());

        file = root.openNextFile();
    }
    Serial.println("");
}
// Handler function for Home page
void handleRoot() {
    digitalWrite(conf["led_pin"].toInt(), 0);

    String out("<h2>Hello from {name}</h2>");
    out += "<h4>Device time: " + conf.getLocalTime() +"</h4>";
    out += "<a href='/cfg'>Edit config</a>";

    out.replace("{name}", "ESP32");

    out += "<script>" + conf.getTimeSyncScript() + "</script>";
    server.send(200, "text/html", out);
    digitalWrite(conf["led_pin"].toInt(), 1);
}

// Handler for page not found
void handleNotFound() {
    digitalWrite(conf["led_pin"].toInt(), 1);
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    digitalWrite(conf["led_pin"].toInt(), 0);
}

// Set time via NTP, as required for x.509 validation
void setClock() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // UTC

    Serial.print(F("Waiting for NTP time sync: "));
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        yield();
        delay(500);
        Serial.print(F("."));
        now = time(nullptr);
    }

    Serial.println(F(""));
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeinfo));
}

// *********** Main application funcions ************
void setup(void) {

    Serial.begin(115200);
    Serial.print("\n\n\n\n");
    Serial.flush();
    LOG_INF("Starting..\n");
    debugMemory("setup");
    //Start local storage
    if(!STORAGE.begin(true)) Serial.println("ESP32 Storage failed!");

    ListDir("/");

    //Initialize ConfigAssist json dictionary pointer
    //If ini file is valid json will not be used
    conf.initJsonDict(appConfigDict_json);

    //Uncomment to remove ini file and re-built it fron dictionary
    //conf.deleteConfig();

    //Failed to load config or ssid empty
    if(!conf.valid() || conf["st_ssid"]=="" ){
        //Start Access point server and edit config
        //Data will be availble instantly 
        conf.setup(server, true);
        return;
    }
    debugMemory("Loaded config");
    pinMode(conf["led_pin"].toInt(), OUTPUT);

    //Connect to Wifi station with ssid from conf file
    uint32_t startAttemptTime = millis();
    WiFi.mode(WIFI_STA);
    LOG_DBG("Wifi Station starting, connecting to: %s\n", conf["st_ssid"].c_str());
    WiFi.begin(conf["st_ssid"].c_str(), conf["st_pass"].c_str());
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000)  {
        digitalWrite(conf["led_pin"].toInt(), 0);
        Serial.print(".");
        delay(50);
        digitalWrite(conf["led_pin"].toInt(), 1);
        delay(500);
        Serial.flush();
    }
    Serial.println();

    //Check connection
    if(WiFi.status() == WL_CONNECTED ){
        LOG_INF("Wifi AP SSID: %s connected, use 'http://%s' to connect\n", conf["st_ssid"].c_str(), WiFi.localIP().toString().c_str());
    }else{
        //Fall back to Access point for editing config
        LOG_ERR("Connect failed.");
        conf.setup(server, true);
        return;
    }

    if (MDNS.begin(conf["host_name"].c_str())) {
        LOG_INF("MDNS responder started\n");
    }

    setClock();

    //Get int/bool value
    bool debug = conf["debug"].toInt();
    LOG_INF("Boolean value: %i\n", debug);

    //Get float value
    float float_value = atof(conf["float_val"].c_str());
    LOG_INF("Float value: %1.5f\n", float_value);

    //Change a value
    //conf.put("led_pin","3");

    //Also change an int/bool value
    //conf.put("led_pin", 4);

    //Register handlers for web server    
    server.on("/", handleRoot);

    //Add handlers to web server 
    conf.setup(server);

    server.on("/inline", []() {
        server.send(200, "text/plain", "this works as well");
        conf.dump();
    });

    server.onNotFound(handleNotFound);
    server.begin();
    LOG_INF("HTTP server started\n");

    //On the fly generate an ini info file on SPIFFS
    {
        if(debug) STORAGE.remove("/info.ini");
        ConfigAssist info("/info.ini");
        //Add a key even if not exists. It will be not editable
        if(!info.valid()){
            info.put("var1", "test1", true);
            info.put("var2", 1234, true);
            info.saveConfigFile();
        }else{
            LOG_DBG("Info file: var1:  %s, var2: %s\n", info["var1"].c_str(), info["var2"].c_str() );
        }
    }
}
// App main loop 
void loop(void) {
    server.handleClient();

    //Display info
    if (millis() - pingMillis >= 10000){
        //if debug is enabled in config display memory debug messages    
        if(conf["debug"].toInt()) debugMemory("Loop");
        pingMillis = millis();
    }

    //allow the cpu to switch to other tasks  
    delay(2);
}