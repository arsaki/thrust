/*  Anna S. a.k.a plokmit
 *  2022
 *  Distributed free
 *  Rocket test rig main program
 */

#include <ESP8266WebServer.h>
#include <HX711.h>

#define DT  		12
#define SCK 		13
#define ARRAY_LENGTH	1024

extern const char logo_jpeg[81312];
extern const char webpage[] PROGMEM;

const char *ssid = "ESPAP"; 
const char *password = "";

const double calibrationCoefficient = 17.31; 
const double unitsToKg = 0.035274
/* Moscow region acceleration */
const double gMoscow = 9.8154;    


int count = 0;
double arr[ARRAY_LENGTH]; 
double newtons; 
double sec = 0;
double impulse = 0;
String measureState ="0"; 

HX711 scale;
ESP8266WebServer server(80); 



void mainHTMLPage()
{
  String s = webpage; /*web.htm*/
  server.send(200, "text/html",s); 
}

void sendImage()
{
  server.send_P(200, "image/jpeg", logo_jpeg, sizeof(logo_jpeg));
}


void setStateMeasure() 
{
  measureState = server.arg("state");
}

void sendArray() 
{
  String arrStr;
  for (int i = 0; i < ARRAY_LENGTH+COUNT; i++)
    arrStr += (String)arr[i] + " ";
  server.send(200,"text/html", arrStr);
}

void sendThrust()
{
  String thrust =(String) (scale.get_units()*unitsToKg*gMoscow/1000);
  server.send(200,"text/plane", thrust);
}

void sendImpulse()
{
  impulse=0;
  for (int i=0; i<ARRAY_LENGTH; i++){
    if((String)arr[i] != "nan"){
      impulse = impulse + (arr[i]*0.1);
    }
  }
  server.send(200,"text/plane", (String)impulse);
}

void sendFile() 
{
  String file;
  for (int i = 0; i < count; i++){
    sec += 0.1;
    file += (String)arr[i] + "," + sec +"," + "\n";
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
    arr[i] = NAN;
  count = 0;
  sec = 0;
}

void setup(void) 
{
  WiFi.softAP(ssid, password);
  Serial.begin(115200);
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  scale.begin(DT, SCK); 
  scale.set_scale();  
  scale.tare();
  scale.set_scale(calibrationCoefficient);
  server.on("/",mainHTMLPage);
  server.on("/logo", sendImage);
  server.on("/state_measure", setStateMeasure);
  server.on("/array", sendArray);
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
  for (int i = 0; (measureState=="1") && (i<ARRAY_LENGTH); i++) {
    if(i == ARRAY_LENGTH - 1)
      measureState = "0";
    delay(87);
    newtons = scale.get_units()*unitsToKg*gMoscow/1000;
    arr[i] = newtons;
    count++;
    server.handleClient();
  }
}
