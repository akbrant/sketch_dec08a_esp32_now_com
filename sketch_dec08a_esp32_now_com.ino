
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

// Replace with your network credentials (STATION)
const char* ssid = "crusher6000";
const char* password = "jamiebrant";

#define BOARD_ID 1

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
int plowRaiseTimer;
int plowRaiseTimerRun;

//Structure of touch sensor values to be received
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

// Create a struct_message to hold incoming sensor readings
struct_touch_message incomingReadings;

JSONVar board;

AsyncWebServer server(80);
AsyncEventSource events("/events");


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
  if (plowRaiseTimerRun == 0) {  //only read timer value if done with raise actions. 
    plowRaiseTimer = incomingReadings.plowRaiseTimer;
    plowRaiseTimerRun = plowRaiseTimer;
  }

  board["id"] = BOARD_ID;
  board["plowLeft"] = incomingReadings.plowLeft;
  board["plowRight"] = incomingReadings.plowRight;
  board["plowRaise"] = incomingReadings.plowRaise;
  board["plowFloat"] = floating;
  board["plowPressure"] = downPressure;
  board["plowRaiseTimer"] = plowRaiseTimerRun;

  
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());
  
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>We Plowing!</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .card.temperature { color: #fd7e14; }
    .card.humidity { color: #1b78e2; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>SNOW WAY ITS WORKING!</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card left">
        <h4><i class="fas fa-thermometer-half"></i>LEFT</h4><p><span class="reading"><span id="t1"></span> </span></p><p class="packet">200 Nominal, 10 working <span id="rt1"></span></p>
      </div>
      <div class="card right">
        <h4><i class="fas fa-tint"></i>RIGHT</h4><p><span class="reading"><span id="h1"></span></span></p><p class="packet"><span id="rh1"></span></p>
      </div>
    
      <div class="card up">
        <h4><i class="fas fa-tint"></i>UP</h4><p><span class="reading"><span id="plowRaise"></span></span></p><p class="packet"><span id="rh1"></span></p>
      </div>
      <div class="card down">
        <h4><i class="fas fa-tint"></i>Down</h4><p><span class="reading"><span id="plowFloat"></span></span></p><p class="packet">1 active, 0 not active<span id="rh1"></span></p>
      </div>

      <div class="card DP">
        <h4><i class="fas fa-tint"></i>Down Pressure</h4><p><span class="reading"><span id="plowDP"></span></span></p><p class="packet">1 active, 0 not active<span id="rh1"></span></p>
      </div>
      <div class="card up timer">
        <h4><i class="fas fa-tint"></i>Raise Timer</h4><p><span class="reading"><span id="plowRaiseTimer"></span></span></p><p class="packet">active when not 0<span id="rh1"></span></p>
      </div>

      
      
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);
  var obj = JSON.parse(e.data);
  document.getElementById("t"+obj.id).innerHTML = obj.plowLeft;
  document.getElementById("h"+obj.id).innerHTML = obj.plowRight;
  document.getElementById("plowFloat").innerHTML = obj.plowFloat;
  document.getElementById("plowRaise").innerHTML = obj.plowRaise;  
  document.getElementById("plowDP").innerHTML = obj.plowPressure;
  document.getElementById("plowRaiseTimer").innerHTML = obj.plowRaiseTimer;
 }, false);
}
</script>
</body>
</html>)rawliteral";


 
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

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
   
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
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
  Serial.print (" Float timer: ");
  Serial.print (plowRaiseTimerRun);
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
  else if ((plowRaiseTimerRun > 0) && Contact)
  {
    floating = 0;
    downPressure = 0;
    plowRaiseTimerRun--;
    Serial.println("plow raise timer");
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
  
  delay (200);
}
