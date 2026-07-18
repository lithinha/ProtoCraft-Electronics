/*
  ESP-NOW Unicast Mesh Node — ProtoCraft Electronics (Part 2)
  Core: ESP32 Arduino Core v3.x

  Flash this SAME sketch to all 3 boards. Each board can independently
  address either of the other two boards by MAC — proving unicast lets
  you choose the recipient, unlike broadcast which reaches everyone.

  Button 0 -> sends a packet to Peer 0 (lights Peer 0's "LED from me")
  Button 1 -> sends a packet to Peer 1 (lights Peer 1's "LED from me")

  On receive, this board checks WHO sent the packet (via src_addr) and
  lights the matching LED — LED_PEER0 if it came from Peer 0's MAC,
  LED_PEER1 if it came from Peer 1's MAC.

  ============================================================
  EDIT THIS BLOCK FOR EACH BOARD — everything else stays identical
  ============================================================
*/

// ---- Step 1: paste the MAC addresses of the OTHER TWO boards here ----
// Get these by flashing get_mac_address.ino to each board first.
uint8_t peerMac0[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // <-- Peer 0's MAC
uint8_t peerMac1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // <-- Peer 1's MAC

// ---- Step 2: uncomment ONE pin block below, matching this board's hardware ----

// -- Classic ESP32 Dev Kit --
#define BUTTON_PEER0 4
#define BUTTON_PEER1 13
#define LED_PEER0    18
#define LED_PEER1    19

// -- ESP32-C3 Super Mini (comment out the block above, uncomment this one instead) --
// #define BUTTON_PEER0 4
// #define BUTTON_PEER1 5
// #define LED_PEER0    6
// #define LED_PEER1    7

/*
  ============================================================
  Nothing below this line needs to change per board.
  ============================================================
*/

#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  bool trigger;
} struct_message;

struct_message outgoingData;

unsigned long led0OnUntil = 0;
unsigned long led1OnUntil = 0;
const unsigned long ledHoldMs = 300;

bool lastButton0State = HIGH;
bool lastButton1State = HIGH;
unsigned long debounce0 = 0, debounce1 = 0;
const unsigned long debounceDelay = 30;

void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Send OK" : "Send failed");
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  struct_message incoming;
  memcpy(&incoming, data, sizeof(incoming));
  if (!incoming.trigger) return;

  if (memcmp(info->src_addr, peerMac0, 6) == 0) {
    digitalWrite(LED_PEER0, HIGH);
    led0OnUntil = millis() + ledHoldMs;
    Serial.println("Packet from Peer 0 -> LED_PEER0 ON");
  } else if (memcmp(info->src_addr, peerMac1, 6) == 0) {
    digitalWrite(LED_PEER1, HIGH);
    led1OnUntil = millis() + ledHoldMs;
    Serial.println("Packet from Peer 1 -> LED_PEER1 ON");
  }
}

void addPeer(uint8_t *mac) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(mac)) {
    esp_now_add_peer(&peerInfo);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PEER0, INPUT_PULLUP);
  pinMode(BUTTON_PEER1, INPUT_PULLUP);
  pinMode(LED_PEER0, OUTPUT);
  pinMode(LED_PEER1, OUTPUT);
  digitalWrite(LED_PEER0, LOW);
  digitalWrite(LED_PEER1, LOW);

  WiFi.mode(WIFI_STA);
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  addPeer(peerMac0);
  addPeer(peerMac1);

  Serial.println("Unicast mesh node ready.");
}

void loop() {
  bool r0 = digitalRead(BUTTON_PEER0);
  if (r0 != lastButton0State) debounce0 = millis();
  if ((millis() - debounce0) > debounceDelay && r0 == LOW) {
    outgoingData.trigger = true;
    esp_now_send(peerMac0, (uint8_t *) &outgoingData, sizeof(outgoingData));
    Serial.println("Sent -> Peer 0");
    delay(200);
  }
  lastButton0State = r0;

  bool r1 = digitalRead(BUTTON_PEER1);
  if (r1 != lastButton1State) debounce1 = millis();
  if ((millis() - debounce1) > debounceDelay && r1 == LOW) {
    outgoingData.trigger = true;
    esp_now_send(peerMac1, (uint8_t *) &outgoingData, sizeof(outgoingData));
    Serial.println("Sent -> Peer 1");
    delay(200);
  }
  lastButton1State = r1;

  if (millis() > led0OnUntil) digitalWrite(LED_PEER0, LOW);
  if (millis() > led1OnUntil) digitalWrite(LED_PEER1, LOW);
}
