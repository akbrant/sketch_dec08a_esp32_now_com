
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>


// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xF0, 0x08, 0xd1, 0xdc, 0x73, 0x74};

//Define the eight GPIO pins for relay board
int motor = 16; 
int valveA = 17; 
int valveB = 5;
int valveC = 18;
int valveD = 19;
int valveE = 21;
int valveF = 22;
int powerDown = 32; 

int CapacitanciaMaxima = 20; // value that gives us the certainty of touch (find that value through calibration) started at 20
int floating = 0;  //track if plow should be locked in float mode. 
int downPressure = 0;  //track if plow should be locked in down pressure mode. 
int Contact = 0;


// Define variables to store incoming readings
int plowLeft;
int plowRight;
int plowRaise;
int plowFloat;
int plowPressure;

//Structure of touch sensor values to be received
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
  Contact = 1;
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

  //setup GPIO pins
  delay (100);
  pinMode (motor, OUTPUT);
  pinMode (valveA, OUTPUT);
  pinMode (valveB, OUTPUT);
  pinMode (valveC, OUTPUT);
  pinMode (valveD, OUTPUT);
  pinMode (valveE, OUTPUT);
  pinMode (valveF, OUTPUT);
  pinMode (powerDown, OUTPUT);

  //set all solnoids to off or high in this case
  digitalWrite (motor, HIGH);
  digitalWrite (valveC, HIGH);
  digitalWrite (valveF, HIGH);
  digitalWrite (valveA, HIGH);
  digitalWrite (valveB, HIGH);
  digitalWrite (valveD, HIGH);
  digitalWrite (valveE, HIGH);
  digitalWrite (powerDown, HIGH);

 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

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
 
  //Serial.println ("Touch Values via esp32 NOW radio:");
  Serial.print ("L: ");
  Serial.print (plowLeft);
  Serial.print (" R: ");
  Serial.print (plowRight);
  Serial.print (" Ra: ");
  Serial.print (plowRaise);
  Serial.print (" F: ");
  Serial.print (plowFloat);
  Serial.print (" DP: ");
  Serial.print (plowPressure);
  Serial.print (" F_state: ");
  Serial.print (floating);
  Serial.print (" DP_state ");
  Serial.print (downPressure);
  Serial.println();

  if ((plowLeft < CapacitanciaMaxima) && Contact)
  {
    Serial.println("plow left");
    digitalWrite (motor, LOW);
    digitalWrite (valveD, LOW);
    if(downPressure) {  //NOTE: Valve E must be deactivated for angle functions
      digitalWrite (valveE, HIGH);
    }
  }
  // check if the average value read on the touchLeft is less than the maximum capacitance
  // if true, this characterizes a touch
  // the red and green LEDs will flash with an interval of 500 ms
  else if ((plowRight < CapacitanciaMaxima) && Contact)
  {
    Serial.println("plow right");
    digitalWrite (motor, LOW);
    digitalWrite (valveA, LOW);
    //digitalWrite (motor,! digitalRead (motor)); // inverts the current state of the LED (HIGH / LOW)
    //digitalWrite (valveA,! digitalRead (valveA)); // inverts the current state of the LED (HIGH / LOW)
    //delay (250);
    if(downPressure) { //NOTE: Valve E must be deactivated for angle functions
      digitalWrite (valveE, HIGH);
    }
  }
  else if ((plowRaise < CapacitanciaMaxima) && Contact)
  {
    floating = 0;
    downPressure = 0;
    Serial.println("plow raise");
    digitalWrite (motor, LOW);
    digitalWrite (valveC, LOW);
    digitalWrite (valveF, LOW);
    //all other valves high
    digitalWrite (valveA, HIGH);
    digitalWrite (valveB, HIGH);
    digitalWrite (valveD, HIGH);
    digitalWrite (valveE, HIGH);
    digitalWrite (powerDown, HIGH);
  }
  else if ((plowFloat < CapacitanciaMaxima) && Contact)
  {
    floating = 1;
    downPressure = 0;
    Serial.println("plow float");
    digitalWrite (valveB, LOW);  
    digitalWrite (valveF, LOW);
    //all other valves high
    digitalWrite (motor, HIGH);      
    digitalWrite (valveA, HIGH);
    digitalWrite (valveC, HIGH);
    digitalWrite (valveD, HIGH);
    digitalWrite (valveE, HIGH);
    digitalWrite (powerDown, HIGH);
    
  }
  else if ((plowPressure < CapacitanciaMaxima) && Contact)
  {
    floating = 0;
    downPressure = 1;
    Serial.println("plow down pressure");
    digitalWrite (powerDown, LOW);
    digitalWrite (valveB, LOW);    
    digitalWrite (valveE, LOW);
    //all other valves high
    digitalWrite (motor, HIGH);      
    digitalWrite (valveA, HIGH);
    digitalWrite (valveC, HIGH);
    digitalWrite (valveD, HIGH);
    digitalWrite (valveF, HIGH);        
  }
  
  // if none of the touch pins are being touched, the set valves to last states
  else {
    digitalWrite (motor, HIGH);
    digitalWrite (valveA, HIGH);    
    digitalWrite (valveC, HIGH);
    digitalWrite (valveD, HIGH);
   
    if (!floating) {  //turn off float valves
      digitalWrite (valveF, HIGH);
    }
    if (!downPressure){ //turn off down pressue
       digitalWrite (powerDown, HIGH);
       digitalWrite (valveE, HIGH);
    } else {
      digitalWrite (valveE, LOW);
    }
    if (!floating && !downPressure){  //turn of shared valve
      digitalWrite (valveB, HIGH);
    }       
  }
  
  delay (100);
}
