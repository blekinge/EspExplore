#include <WiFiProv.h>

void SysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id)
    {
    case ARDUINO_EVENT_WIFI_READY:
        log_d("Wifi is ready");
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        log_d("Disconnected. Connecting to the AP again... ");
        break;

    case ARDUINO_EVENT_PROV_INIT:
        log_d("Provisioning initialised");    
        break;
    case ARDUINO_EVENT_PROV_START:
        log_d("Provisioning started\nGive Credentials of your access point using \" Android app \"");
        break;

    case ARDUINO_EVENT_PROV_CRED_RECV:
    {
        log_d("Received Wi-Fi credentials");
        log_d("SSID : %s", (const char *)sys_event->event_info.prov_cred_recv.ssid);
        log_d("Password : %s", (char const *)sys_event->event_info.prov_cred_recv.password);
        break;
    }

    case ARDUINO_EVENT_PROV_CRED_FAIL:
    {
        log_w("Provisioning failed!");
        if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR)
        {
            log_w("Wi-Fi AP password incorrect. Resetting to factory and retrying provisioning");
            wifi_prov_mgr_reset_provisioning();
        }
        else
        {
            log_w("Wi-Fi AP not found....");
        }
        break;
    }

    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        log_d("Provisioning Successful");
        break;

    case ARDUINO_EVENT_PROV_END:
        log_d("Provisioning Ends");
        esp_restart();
        break;

    case ARDUINO_EVENT_WIFI_STA_START:
        log_d("Attempting to connect to wifi");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        log_d("Connected to wifi '%s'", sys_event->event_info.wifi_sta_connected.ssid);
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        log_d("Connected IP address : %s", IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr).toString());
        break;

    default:
        break;
    }
}

void provisionWithSoftAP(String softApName, String pop)
{
    WiFi.onEvent(SysProvEvent);
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP,
                            WIFI_PROV_SCHEME_HANDLER_NONE,
                            WIFI_PROV_SECURITY_1,
                            pop.c_str(),
                            softApName.c_str());

    while (!WiFi.isConnected()){
        delay(1000);
    }
    log_i("IP address: %s", WiFi.localIP().toString());
}