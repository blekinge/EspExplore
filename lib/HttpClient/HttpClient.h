
#ifndef Http_h
#define Http_h
#include <HTTPClient.h>

struct header_t {
  const String name;
  const String value;
};

typedef enum {
  BASIC
} auth_types_t;

struct endpoint_t {
  String httpMethod;
  String url;
  String body;
  header_t headers[4];
  uint8_t count;
  String username;
  String password;
  auth_types_t authMethod;
};
#endif


//TODO https

String httpGetRequest(endpoint_t endpoint);
