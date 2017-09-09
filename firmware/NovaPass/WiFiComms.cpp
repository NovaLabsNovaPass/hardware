#include <Arduino.h>
#include "NovaPass.h"
#include <WiFi101.h>
#include "WiFiComms.h"

// TODO: RestClient is using a different WifClient object. 
// needs to be merged
#if defined(NOPA_USING_SSL)
static WiFiSSLClient client;
#else
static WiFiClient client;
#endif

void WiFiComms::Setup() {
  status = WL_IDLE_STATUS;
  got_disconnected = false;
}

void WiFiComms::Connect() {
  uint8_t i = 0;
  
  // attempt to connect to WiFi network:
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(network_name, network_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    SerialUSB.println("Waiting to connect..");
    if (++i > 10)
      break;
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

}

#if 0

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 443)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  }
#endif
