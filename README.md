# TODO

## ~~Provisioning~~

You can WIFI-provision in a number of ways

### Console

From <https://github.com/espressif/esp-idf> you can get the tool `esp_prov`

When the device boots up, it creates a new AP, named `Prov_123`

Connect to this AP and run

```bash
/home/abr/Arduino/esp/esp-idf/tools/esp_prov/esp_prov.py \
    --transport 'softap' \
    --service_name '192.168.4.1' \
    --sec_ver 1 \
    --pop abcd1234 \
    --ssid 'SSID' \
    --passphrase 'SSID_PASSWORD'
```

If `esp_prov.py` gives errors about python or modules, do

```bash
cd /home/abr/Arduino/esp/esp-idf/
./install.sh
source ~/.espressif/python_env/idf5.0_py3.11_env/bin/activate
pip install bleak future protobuf=4.21 cryptography
```

## ~~OTA~~

Generate certificates with

```bash
openssl req -new -x509 -keyout OTAserver/src/main/resources/server.pem -out lib/Ota/server.crt -days 365 -nodes  -subj "/CN=$(hostname)"
```

Run the java (quarkus) server with `mvn quarkus:run`
It serves the firmware file from `.pio/build/az-delivery-devkit-v4/firmware.bin`

The OTA client sends current md5 and sha1. If md5 matches current file, the OTA server sends HTTP NotModified

OTA server is protected with basic auth, `username@password`

There are still spurious ssl timeout errors; probably related to <https://github.com/espressif/arduino-esp32/issues/7057>

## <https://github.com/plerup/makeEspArduino/blob/master/README.md>

## Preferences api
