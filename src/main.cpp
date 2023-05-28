// This sets up Logging. 4 i DEBUG, 3 is INFO
#define CORE_DEBUG_LEVEL 4
#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif

#include <Arduino.h>

#include "config.h"
#include <Preferences.h>

#include <HttpsOTAUpdate.h>
#include <ESPmDNS.h>

Preferences prefs;

static const char *url = "https://gameon.askov.net:4443/.pio/build/az-delivery-devkit-v4/firmware.bin"; // state url of your firmware image

static const char *server_certificate = "-----BEGIN CERTIFICATE-----\n" \
                                        "MIIDZTCCAk2gAwIBAgIUeA5ALB1jlZBVs1j2HlesT2PpGjIwDQYJKoZIhvcNAQEL\n" \
                                        "BQAwQjELMAkGA1UEBhMCWFgxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UE\n" \
                                        "CgwTRGVmYXVsdCBDb21wYW55IEx0ZDAeFw0yMzA1MjgxMjIzMzZaFw0yNDA1Mjcx\n" \
                                        "MjIzMzZaMEIxCzAJBgNVBAYTAlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAa\n" \
                                        "BgNVBAoME0RlZmF1bHQgQ29tcGFueSBMdGQwggEiMA0GCSqGSIb3DQEBAQUAA4IB\n" \
                                        "DwAwggEKAoIBAQClD+GGysBDHMwYbV6PPcmGuWF55uE7GoSodXf8/6yK55yU1T8q\n" \
                                        "q2LXziFZDjiUe56JB1b2CXlhNqwDeVcrLsI48eAAboIfqnKBPi3XDlqgXLD4t1Xi\n" \
                                        "c7o7qG7xOL7sWMEjC+ev5cWIB6KovT22Mr47lxaobPMVHutIN5nXPDFIsnvJjRQG\n" \
                                        "QcFlIHfZlvxhTC2BujLTINA3vQXNwMMjhrmGx0uTbDe0XzzZurz0mSkqUQr4VpoZ\n" \
                                        "Ebq/aNRIXerG9lfU+DZtgyYeuDa3s8Z2p1rl8JTN9ilDDOG7gfJvwWoTVOgt8XJl\n" \
                                        "eXtXKGg0pV2XlQ2Qhts3bWxRo2tMjpS/SxHtAgMBAAGjUzBRMB0GA1UdDgQWBBSX\n" \
                                        "r/LdHtne6rqwdQ0N8iB89nt1MTAfBgNVHSMEGDAWgBSXr/LdHtne6rqwdQ0N8iB8\n" \
                                        "9nt1MTAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQALMdSq98V+\n" \
                                        "D0K+1erbVvb/1rIx6IlXVBpsxBItIK25H9ShftJGZRkbbknONCEqXvb908z/prKA\n" \
                                        "CsNVmG6CYZZMzzRUGNbRjBwVPavX9vc1xPtQl5aNusNBznER6Uy+sbDtuUHkq27j\n" \
                                        "hpS7jgikQwVsqzGjim/kdMdk9S5D9a0huLefS3+NUDRB4OsSxxaBYdJwfLw+kWrw\n" \
                                        "rPno5YjYx+CoLppLO9xfVOuakymPZ1/aPh+d+nPQ2IP/ekLJ+aTLRtJTDb9LQ0U3\n" \
                                        "ZhFowwH7ff88fUVIdhnQEiARLKp5RVGSTIN0vUZ9aSjV+qDmjpO1q/v5v1Ak0KzZ\n" \
                                        "P6H51TTgGNql\n" \
                                        "-----END CERTIFICATE-----";

static HttpsOTAStatus_t otastatus;

void HttpEvent(HttpEvent_t *event)
{
  switch (event->event_id)
  {
  case HTTP_EVENT_ERROR:
    log_e("Http Event Error");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    log_i("Http Event On Connected");
    break;
  case HTTP_EVENT_HEADER_SENT:
    log_d("Http Event Header Sent");
    break;
  case HTTP_EVENT_ON_HEADER:
    log_d("Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    break;
  case HTTP_EVENT_ON_FINISH:
    log_i("Http Event On Finish");
    break;
  case HTTP_EVENT_DISCONNECTED:
    log_i("Http Event Disconnected");
    break;
  }
}

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

  HttpsOTA.onHttpEvent(HttpEvent);
  log_i("Starting OTA");
  HttpsOTA.begin(url, server_certificate);

  log_i("Please Wait it takes some time ...");
}

void loop()
{
  otastatus = HttpsOTA.status();
  if (otastatus == HTTPS_OTA_SUCCESS)
  {
    log_i("Firmware written successfully. To reboot device, call API ESP.restart() or PUSH restart button on device");
  }
  else if (otastatus == HTTPS_OTA_FAIL)
  {
    log_w("Firmware Upgrade Fail");
  }
  delay(1000);
}
