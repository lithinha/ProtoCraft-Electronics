/*
  Get MAC Address — ProtoCraft Electronics
  Flash this to each of your 3 boards ONE AT A TIME, open Serial Monitor
  at 115200 baud, and note down the printed MAC address for each board.
  You'll hardcode these into the peer lists of the OTHER two boards
  in the main unicast_mesh_node sketch.
*/

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("This board's MAC address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // nothing to do — just read the Serial Monitor output
}
