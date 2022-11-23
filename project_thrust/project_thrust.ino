/*  
 *  Author Anna S. a.k.a plokmit
 *  2022
 *  Distributed free
 *  Rocket engines thrust stand program
 */

#include <ESP8266WebServer.h>
#include <HX711.h>

/* NodeMCU pin assignment */
#define DT  		12
#define SCK 		13
/* Thrust measurement data array length */
#define ARRAY_LENGTH	  1024
#define LOGO_JPEG_SIZE  81312

/* logo_thrust.ino */
extern const char logo_jpeg[LOGO_JPEG_SIZE];
/* web.htm */
extern const char webpage[];

const char *ssid     = "Экспансия"; 
const char *password = "";

/* 5kg load cell */
const double calibrationCoefficient = 17.233826;
const double unitsToKg = 0.035274;
/* Gravitational acceleration in Moscow region */
const double g = 9.8154;    

double newtons = 0; 
/* Thrust measurement data array */
double thrustArray[ARRAY_LENGTH]; 
int    thrustArrayCount   = 0;
time_t startTime = 0;
String measureState ="0"; 

HX711 scale;
ESP8266WebServer server(80); 



void mainHTMLPage()
{
  server.send(200, "text/html",webpage); 
}

void sendImage()
{
  server.send_P(200, "image/jpeg", logo_jpeg, sizeof(logo_jpeg));
}


void setStateMeasure() 
{
  measureState = server.arg("state");
}

void sendThrust()
{
  String thrust;
  if (measureState == "0")
    thrust = (String)(scale.get_units()*unitsToKg*g/1000);
  else
    thrust = (String)newtons;
  server.send(200, "text/plane", thrust);
}

void sendImpulse()
{
  double impulse = 0;
  for (int i = 0; i < ARRAY_LENGTH; i++){
    if((String)thrustArray[i] != "nan"){
      impulse = impulse + (thrustArray[i]*0.1);
    }
  }
  server.send(200, "text/plane", (String)impulse);
}

void sendFile() 
{
  String file;
  double sec = 0;
  for (int i = 0; i < thrustArrayCount; i++){
    sec += 0.1;
    file += (String)thrustArray[i] + "," + sec +"," + "\n";
  }
  server.send(200,"text/csv", file);
}

void calibrate()
{
  scale.tare();
}

void clearArray() 
{
  for (int i = 0; i < ARRAY_LENGTH; i++)
    thrustArray[i] = NAN;
  thrustArrayCount = 0;
}

void setup(void) 
{
  WiFi.softAP(ssid, password);
  Serial.begin(115200);
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  scale.begin(DT, SCK); 
  scale.set_scale(calibrationCoefficient);
  scale.tare();
  server.on("/",mainHTMLPage);
  server.on("/logo", sendImage);
  server.on("/state_measure", setStateMeasure);
  server.on("/thrust", sendThrust);
  server.on("/impulse", sendImpulse);
  server.on("/file.csv",sendFile);
  server.on("/calibrate", calibrate);
  server.begin(); 
  clearArray();
}

void loop() 
{
  server.handleClient(); 
  if (measureState == "1")
    clearArray();
  for (int i = 0; (measureState == "1") && (i < ARRAY_LENGTH); i++) {
    if(i == (ARRAY_LENGTH - 1))
      measureState = "0";
    startTime = millis();
    Serial.println((double)i/10);
    newtons = scale.get_units()*unitsToKg*g/1000;
    thrustArray[i] = newtons;
    thrustArrayCount++;
    while (millis() < (startTime + 80))
      server.handleClient();
    while (millis() < (startTime + 100));
  }
}
