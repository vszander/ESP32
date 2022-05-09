/* 
**  Connect the ESP8266 unit to an existing WiFi access point
**  For more information see http://42bots.com
*/


// https://randomnerdtutorials.com/esp8266-nodemcu-http-get-post-arduino/
//  https://techtutorialsx.com/2017/04/29/esp32-sending-json-messages-over-mqtt/

// It is a support and diagnostic sketch for the TFT_eSPI library:
// https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#include "DHT.h"

//#include <ESP8266WiFi.h>
  
#include "WiFi.h"
#include <ArduinoJson.h>  //https://arduinojson.org/
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "Daytona"; //replace this with your WiFi network name
const char* password = "0000111182"; //replace this with your WiFi network password
bool connected = 0;

const char* mqttServer = "192.168.176.9";
const int mqttPort = 1883;
const char* mqttUser = "yourInstanceUsername";
const char* mqttPassword = "yourInstancePassword";
const char* TOPIC = "weather/KJCA";

float celcius_float = 25.31;
float far_float = 107.31;
char far_temp[] = "87.32";
int sensorValue = 123; //
float voltage = 1.23;
float pH = 7.11;

//   pH Gravity  adjustments
//   pH = voltage * 3.5 + scale1;
float conversion1 = 3.5;
float scale1 = 2.1;
// pH Graph preferences
 float minpH = 5.6;
 float maxpH = 9;

// Atlas ORP
char computerdata[20];           //we make a 20 byte character array to hold incoming data from a pc/mac/other.
byte received_from_computer = 0; //we need to know how many characters have been received.
byte serial_event = 0;           //a flag to signal when data has been received from the pc/mac/other.
byte code = 0;                   //used to hold the I2C response code.
char ORP_data[20];               //we make a 20 byte character array to hold incoming data from the ORP circuit.
byte in_char = 0;                //used as a 1 byte buffer to store inbound bytes from the ORP Circuit.
byte q = 0;                      //counter used for ORP_data array.
int time_ = 815;                 //used to change the delay needed depending on the command sent to the EZO Class ORP Circuit.
float ORP_float;  //float var used to hold the float value of the ORP.
//int time_ = 815;             //if a command has been sent to calibrate or take a reading we wait 815ms so that the circuit has time to take the reading.
int maxHigh = 630;  //  this is the upper end of the barGraph or 'too much chlorine' ?


float Fahrenheit = 33.44;
float cel;  // readings in celcius 
float humidity2 = 0.1;
 
 int maxBarGraph = 127;
 int indicatorWidth = 10;
  float h2otemp = 55.4;


 
 float percent = (maxpH - pH)/ (maxpH - minpH);
 int xReading = maxBarGraph - (percent * maxBarGraph); 
 




//  Water temp DS18B20  uses one-wire and Dallas Temp Control
const int ONE_WIRE_BUS = 13;  //  37; 


/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature WaterTemp(&oneWire);


#define DHT11PIN 2
#define DHTTYPE DHT11 
DHT myDHT(DHT11PIN, DHTTYPE);




int  delay1 = 1200;

void callback(char* topic, byte* payload, unsigned int length){
    payload[length] = '\0';
    int value = String((char*) payload).toInt();
 
    Serial.println(topic);
    Serial.println(value);
}
WiFiClient espClient;
PubSubClient client(mqttServer,mqttPort,callback,espClient);

void setup() {
  Serial.begin(9600);
  WaterTemp.begin();  //Dallas water temp
  myDHT.begin();
  Serial.println();
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  tft.init();
   tft.setRotation(3);  //2=180   //3=  preferred - on its side  USB to the left
  tft.fillScreen(TFT_BLACK);
 // tft.drawString("Connecting to Wifi", 10, 120 );
    tft.setCursor(0, 0, 2); ///    // Set "cursor" at top left corner of display (0,0) and select font 2
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(2);
  tft.print("Connecting to: ");
  tft.println(ssid);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
    tft.print(".");
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(400);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);
  }
  Serial.print("Connected, IP address: ");
    tft.fillScreen(TFT_BLACK);
   tft.setCursor(0, 0, 2); ///    // Set "cursor" at top left corner of display (0,0) and select font 2
  tft.print("Connected, IP address: ");
  tft.println(WiFi.localIP());
  connected = 1;
  delay1 = 1000;
  Serial.println(WiFi.localIP());
 // client.setServer(mqttServer);
  client.setCallback(callback);
  
  connectMQTT();
  delay(1000);
  tft.fillScreen(TFT_BLACK);
}



void loop() {
  // put your main code here, to run repeatedly:
     digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(delay1);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

//   get sensor stuff
  getWaterTemp();
  getDHT11();

//update the screen

//  tft.fillScreen(TFT_BLACK);
 // tft.drawString("Connecting to Wifi", 10, 120 );
    tft.setCursor(0, 0, 2); ///    // Set "cursor" at top left corner of display (0,0) and select font 2
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(2);

  tft.print("WaterTemp:");
  tft.println(far_float);


  StaticJsonDocument<300> JSONdoc;
//  JsonObject& JSONencoder = JSONdoc.createObject();   //not needed in ArduinoJson 6


  
  JSONdoc["tubwatertempF"] = far_float;
  JSONdoc["humidity"] = humidity2;
  JSONdoc["tempf"] = Fahrenheit;
  
  JSONdoc["station"] = "ESP8266";
 // JsonArray values = JSONdoc.createNestedArray("values");
   
//  values.add(20);
//  values.add(21);
//  values.add(23);//  topic weather/KJCA

 // char JSONmessageBuffer[100];
 // JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer))

  //Serial.println(JSONmessageBuffer);
  serializeJsonPretty(JSONdoc, Serial); 
  char buff1[256];
  size_t n = serializeJson(JSONdoc, buff1);  //  https://arduinojson.org/v6/how-to/use-arduinojson-with-pubsubclient/
  
  
  if (client.publish("weather/KJCA", buff1, n) == true) {
      Serial.println("Success sending message");
  } else {
      Serial.println("Error sending message");
  }
   
  client.loop();
  Serial.println("-------------");
 
  delay(10000);

}

float getWaterTemp(){
  //Serial.print(" Requesting temperatures..."); 
  //float temp= 33.4;
  WaterTemp.requestTemperaturesByIndex(0); // Send the command to get temperature readings 
   h2otemp = (9*WaterTemp.getTempCByIndex(0) / 5 ) + 32;   // Why "byIndex"? 
   h2otemp = WaterTemp.getTempFByIndex(0); 
   // You can have more than one DS18B20 on the same bus.  
   // 0 refers to the first IC on the wire 
 Serial.print("Water Temperature is: "); 
     tft.setCursor(0, 16, 2); ///    // Set "cursor" at top left corner of display (0,0) and select font 2
   //  h2otemp = 55.6;
tft.print("Water Temp: "); 
 tft.println(h2otemp);
  Serial.println(h2otemp); 

    /*    do we want to display water temp   ??
  //display.setCursor(4,8);
     //display.setTextSize(2);
 //display.print(far_float, 1);
 
*/

  far_float = h2otemp;
  return h2otemp;
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266")) {
 
      Serial.println("connected");
       tft.println("MQTT connected");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
}

void getDHT11() {
//  https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/
    Serial.print("Humidity (%): ");
  humidity2 = myDHT.readHumidity();
  Serial.println(humidity2, 2);
  cel = (float)myDHT.readTemperature();
  Fahrenheit = (9 * cel /5) + 32;
  int Fahren2 = Fahrenheit;
  float f = myDHT.readTemperature(true);
  
    // Check if any reads failed and exit early (to try again).
 //   if (isnan(humidity2) || isnan(cel) || isnan(f)) {
  if (isnan(humidity2) || isnan(cel) ) {
    tft.setCursor(0, 32, 2); ///    // Set "cursor" at top left corner of display (0,0) and select font 2
  
       tft.print("DFT sensor fail!");
       delay(2500);
       getDHT11();
    }
   
    Serial.println((float)Fahrenheit, 2);

   tft.setCursor(0, 32, 2); ///    // Set "cursor" at top left corner of display (0,0) and select font 2
   tft.print("Room temp:");
   tft.println(Fahrenheit);
    tft.setCursor(0, 68, 2); ///    // Set "cursor" at top left corner of display (0,0) and select font 2
   tft.print("Humidity:");
   tft.println(humidity2);
 }
