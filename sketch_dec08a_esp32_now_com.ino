
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>


// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xF0, 0x08, 0xd1, 0xdc, 0x73, 0x74};

// Define variables to store incoming readings
int plowLeft;
int plowRight;
int plowRaise;
int plowFloat;
int plowPressure;

//Structure of touch sensor values to send
typedef struct struct_touch_message {
    int plowLeft;
    int plowRight;
    int plowRaise;
    int plowFloat;
    int plowPressure;
} struct_touch_message;

//Create a sttuck_touch_message to hold touch readings
struct_touch_message TouchReadings;

// Create a struct_message to hold incoming sensor readings
struct_touch_message incomingReadings;

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  plowLeft = incomingReadings.plowLeft;
  plowRight = incomingReadings.plowRight;
  plowRaise = incomingReadings.plowRaise;
  plowFloat = incomingReadings.plowFloat;
  plowPressure = incomingReadings.plowPressure;
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
   
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

  Serial.println ("Touch Values via esp32 NOW radio:");
  Serial.println (plowLeft);
  Serial.println (plowRight);
  Serial.println (plowRaise);
  Serial.println (plowFloat);
  Serial.println (plowPressure);
  Serial.println();

  delay(100);
}
