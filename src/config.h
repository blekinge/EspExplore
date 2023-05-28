
// MPU
#include <Adafruit_MPU6050.h>
const mpu6050_accel_range_t accel_range = MPU6050_RANGE_8_G;
const mpu6050_gyro_range_t gyro_range = MPU6050_RANGE_500_DEG;
const mpu6050_bandwidth_t bandwidth = MPU6050_BAND_21_HZ;

// WIFI
//TODO this should NOT be hardcoded here
const char* ssid     = "";
const char* password = "";
const char* host = "esp32-webupdate";

// HTTP Client
#include <HttpClient.h>
const endpoint_t urls[] = {
  //side1
  { "GET",
    "http://192.168.2.136:8080/side/1",
    "", //body
    {}, //headers
    0, //headers count
    "username", //httpBasic username
    "password", //httpBasic password
    BASIC //authMethod
  },

  //side2
  { "GET",
    "http://192.168.2.136:8080/side/2",
    "", //body
    {}, //headers
    0, //headers count
    "username", //httpBasic username
    "password", //httpBasic password
    BASIC //authMethod
  },

  //side3
  { "GET",
    "http://192.168.2.136:8080/side/3",
    "", //body
    {}, //headers
    0, //headers count
    "username", //httpBasic username
    "password", //httpBasic password
    BASIC //authMethod
  },

  //side4
  { "GET",
    "http://192.168.2.136:8080/side/4",
    "", //body
    {}, //headers
    0, //headers count
    "username", //httpBasic username
    "password", //httpBasic password
    BASIC //authMethod
  },

  //side5
  { "GET",
    "http://192.168.2.136:8080/side/5",
    "", //body
    {}, //headers
    0, //headers count
    "username", //httpBasic username
    "password", //httpBasic password
    BASIC //authMethod
  },

  //side6
  { "GET",
    "http://192.168.2.136:8080/side/6",
    "", //body
    {}, //headers
    0, //headers count
    "username", //httpBasic username
    "password", //httpBasic password
    BASIC //authMethod
  }

};
