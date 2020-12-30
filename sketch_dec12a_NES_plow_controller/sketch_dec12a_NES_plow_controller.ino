#include "GameControllers.h"
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xF0, 0x08, 0xd1, 0xd1, 0x88, 0xEC};

// Variable to store if sending data was successful
String success;


//shared pins between all controllers
const int LATCH_PIN = 16;
const int CLOCK_PIN = 17;
//individual data pin for each controller
const int DATA_PIN_0 = 18;  

GameControllers controllers;

//Structure of touch sensor values to send
typedef struct struct_touch_message {
    int plowLeft;
    int plowRight;
    int plowRaise;
    int plowFloat;
    int plowPressure;
    int plowRaiseTimer;
} struct_touch_message;

//Create a sttuck_touch_message to hold touch readings
struct_touch_message TouchReadings;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? " Delivery Success " : " Delivery Fail ");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}



void setup()
{
  Serial.begin(115200); //prepare serial for text output 
  Serial.println(WiFi.macAddress());
  
  //initialize shared pins
  controllers.init(LATCH_PIN, CLOCK_PIN); 
  //activate first controller ans set the type to SNES
  controllers.setController(0, GameControllers::NES, DATA_PIN_0);

     
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

}

  int plowLeft = 0;
  int plowRight = 0;
  int plowRaise = 0;
  int plowFloat = 0;
  int plowPressure = 0;
  int plowRaiseTimer = 0;
  int loopcount = 0;
void loop()
{
  loopcount ++;
  controllers.poll(); //read all controllers at once
  if(controllers.down(0, GameControllers::START))  //check if Start was pressed since last loop
    Serial.println("Start was pressed.");
  if(controllers.down(0, GameControllers::SELECT))  //check if Start was pressed since last loop
    Serial.println("Select was pressed.");
    
 if(controllers.down(0, GameControllers::B)) {  //if B button is hold down 
    Serial.println("B DP");
    plowPressure = 10;
  } else {
    plowPressure =200;
  }

 if(controllers.down(0, GameControllers::A)) {  //if A button is hold down
    Serial.println("A FLOAT TIMER");
    plowRaiseTimer = 8;
     } else {
    plowRaiseTimer = 0;
  } 

  if(controllers.down(0, GameControllers::UP)) {  //check if button it's currently pressed down
    Serial.println("DOWN FLOAT");
    plowFloat = 10;
  } else {
    plowFloat =200;
  }
  
  if(controllers.down(0, GameControllers::DOWN)) {  //check if button it's currently pressed down
    Serial.println("UP");
    plowRaise = 10;
  } else {
    plowRaise =200;
  } 
     
  if(controllers.down(0, GameControllers::LEFT)) {  //check if B button it's currently pressed down
    Serial.println("LEFT");
    plowLeft = 10;
  } else {
    plowLeft =200;
  }
  if(controllers.down(0, GameControllers::RIGHT)) {  //check if B button it's currently pressed down
    Serial.println("RIGHT");
    plowRight = 10;
  } else {
    plowRight =200;
  }


  // Set values to send
  TouchReadings.plowLeft = plowLeft;
  TouchReadings.plowRight = plowRight;
  TouchReadings.plowRaise = plowRaise;
  TouchReadings.plowFloat = plowFloat;
  TouchReadings.plowPressure = plowPressure;
  TouchReadings.plowRaiseTimer = plowRaiseTimer;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &TouchReadings, sizeof(TouchReadings));
   
  if (result == ESP_OK) {
    if (loopcount % 1000) {
       Serial.println(" Sent OK ");
    }   
  }
  else {
    Serial.println("Error sending the data");
  }
      
  delay(50);  //read controller just ~20times a second
}
