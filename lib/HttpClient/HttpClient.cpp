
#include <HTTPClient.h>
#include <HttpClient.h>


//TODO https

String httpGetRequest(endpoint_t endpoint) {

  //https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/src/HTTPClient.h
  HTTPClient http;

  // configure traged server and url
  //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
  http.begin(endpoint.url); //HTTP

  if (endpoint.authMethod == BASIC) {
    http.setAuthorization(endpoint.username.c_str(), endpoint.password.c_str());
    http.setAuthorizationType("BASIC");
  }

  for (int headerNum = 0; headerNum < endpoint.count; headerNum++) {
    http.addHeader(endpoint.headers[headerNum].name, endpoint.headers[headerNum].value);
  }

  // start connection and send HTTP header
  int httpCode = http.sendRequest(endpoint.httpMethod.c_str(), endpoint.body);

  String result = http.getString();

  http.end();

  return result;

}
