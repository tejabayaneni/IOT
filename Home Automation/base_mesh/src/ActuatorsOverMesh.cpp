
//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a another network and broadcast message from a webpage to the edges of the mesh network.
// This sketch can be extended further using all the abilities of the AsyncWebserver library (WS, events, ...)
// for more details
// https://gitlab.com/painlessMesh/painlessMesh/wikis/bridge-between-mesh-and-another-network
// for more details about my version
// https://gitlab.com/Assassynv__V/painlessMesh
// and for more details about the AsyncWebserver library
// https://github.com/me-no-dev/ESPAsyncWebServer
//************************************************************

#include "painlessMesh.h"
#include "SPI.h"

#define   RELAY_CONTROL_PIN  22
#define   LED_OUTPUT_PIN  27

#define   MESH_PREFIX     "IOT2019"
#define   MESH_PASSWORD   "csueastbay"
#define   MESH_PORT       5555

#define HOSTNAME "HTTP_BRIDGE"

String lastmsg;
// Prototype
void receivedCallback( const uint32_t &from, const String &msg );
IPAddress getlocalIP();

painlessMesh  mesh;

int lastMotionDetectedTime = 0;

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_CONTROL_PIN, OUTPUT);
  pinMode(LED_OUTPUT_PIN, OUTPUT);
  digitalWrite(LED_OUTPUT_PIN, HIGH);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  
  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);
  mesh.setHostname(HOSTNAME);
}

void loop() {
  mesh.update();
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  lastmsg = msg;

  if (msg.equals("Motion = 1")) {
    digitalWrite(RELAY_CONTROL_PIN, HIGH);
    lastMotionDetectedTime = millis();
  } else if (msg.equals("Motion = 0")) {
    int time = millis();

    // Turn off the light 5 seconds after motion is last detected.
    if (abs(lastMotionDetectedTime - time) >= 5000) {
      digitalWrite(RELAY_CONTROL_PIN, LOW);
    }
  }
}
