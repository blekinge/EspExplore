#include <WiFi.h>

#define NO_GLOBAL_HTTPUPDATE 1
#include <HTTPClient.h>
#include <HTTPUpdate.h>

#include <time.h>

#define LED_BUILTIN 2

// Set time via NTP, as required for x.509 validation
void setClock()
{
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // UTC

  Serial.print(F("Waiting for NTP time sync: "));
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
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

// https://docs.platformio.org/en/latest/platforms/espressif32.html#embedding-binary-data
extern const char rootCACertificate[] asm("_binary_lib_Ota_server_crt_start");

void otaUpdate(String otaUrl)
{
  setClock();

  WiFiClientSecure client;

  client.setCACert(rootCACertificate);
  log_d("certificate %s", rootCACertificate);

  // Reading data over SSL may be slow, use an adequate timeout
  client.setTimeout(120); // timeout argument is defined in seconds for setTimeout
  client.setHandshakeTimeout(120 * 1000);
  HTTPClient http;
  http.setTimeout(30 * 1000);
  http.setConnectTimeout(120 * 1000);
  http.begin(client, otaUrl);
  HTTPUpdate httpUpdate = HTTPUpdate(120 * 1000);

  httpUpdate.rebootOnUpdate(true);

  // t_httpUpdate_return ret = httpUpdate.update(http);

  String currentVersion = "currentVersion";
  HTTPUpdateRequestCB requestCB = [](HTTPClient *client) { client->setAuthorization("username", "password"); };
  t_httpUpdate_return ret = httpUpdate.update(http, currentVersion, requestCB);

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    log_e("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    // TODO retry
    break;

  case HTTP_UPDATE_NO_UPDATES:
    log_i("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    log_i("HTTP_UPDATE_OK");
    break;
  }
}
