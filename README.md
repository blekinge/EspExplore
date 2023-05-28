# Provisioning

You can WIFI-provision in a number of ways

## Console

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

