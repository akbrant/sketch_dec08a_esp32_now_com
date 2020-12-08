
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>


// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xF0, 0x08, 0xd1, 0xd1, 0x88, 0xEC};

// Define variables to store BME280 readings to be sent
float temperature;
float humidity;
float pressure;

// Define variables to store incoming readings
float incomingTemp;
float incomingHum;
float incomingPres;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    float temp;
    float hum;
    float pres;
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message BME280Readings;

//Setup the touch pins
int touchLeft = 12; //no 12 // pin with touch sensor (native constant T0 can be used)
int touchRight = 2; //work 2 //no 4  pin with touch sensor (native T4 constant can be used)
int touchRaise = 13; //works
int touchFloat = 32; //32 works //no 14
int touchPressure = 33; //33 works //no 15;

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
struct_message incomingReadings;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? " Delivery Success " : " Delivery Fail ");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingTemp = incomingReadings.temp;
  incomingHum = incomingReadings.hum;
  incomingPres = incomingReadings.pres;
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

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
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

  Serial.print("Touch Values: L ");
  Serial.print (touchRead (touchLeft));
  Serial.print(" R: ");
  Serial.print (touchRead (touchRight));
  Serial.print(" Ra: ");
  Serial.print (touchRead (touchRaise));
  Serial.print(" F: ");
  Serial.print (touchRead (touchFloat));
  Serial.print(" P: ");
  Serial.print (touchRead (touchPressure));
  Serial.println();
  
  int plowLeft = 0;
  int plowRight = 0;
  int plowRaise = 0;
  int plowFloat = 0;
  int plowPressure = 0;

  
  
  // takes 100 readings from each touch sensor and averages the read value
  for (int i = 0; i <100; i ++)
  {
    plowLeft += touchRead (touchLeft);
    plowRight += touchRead (touchRight);
    plowRaise += touchRead (touchRaise);
    plowFloat += touchRead (touchFloat);
    plowPressure += touchRead (touchPressure);

  }
   
  plowLeft = plowLeft / 100;
  plowRight = plowRight / 100;
  plowRaise = plowRaise / 100;
  plowFloat = plowFloat / 100;
  plowPressure = plowPressure / 100;

 
 
  // Set values to send
  TouchReadings.plowLeft = plowLeft;
  TouchReadings.plowRight = plowRight;
  TouchReadings.plowRaise = plowRaise;
  TouchReadings.plowFloat = plowFloat;
  TouchReadings.plowPressure = plowPressure;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &TouchReadings, sizeof(TouchReadings));
   
  if (result == ESP_OK) {
    Serial.print(" Sent OK ");
  }
  else {
    Serial.println("Error sending the data");
  }

  delay(100);
}
