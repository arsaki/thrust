#include <ESP8266WebServer.h>
#include <HX711.h>

#define DT  12
#define SCK 13

extern const char logo_jpeg[81312];
extern const char webpage[] PROGMEM;
int count=0;
double arr[1024]; 
double newtons; 
double calibrationCoefficient=17.31; 
double sec = 0;
double impulse = 0;
String measureState="0"; 
const char *ssid = "ESPAP"; 
const char *password = "password";

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
    for (int i = 0; i < 1024; i++)
      arrStr +=(String)arr[i] + " ";
    server.send(200,"text/html", arrStr);
}

void sendThrust()
{
  String thrust =(String) (scale.get_units()* 0.035274*9.8/1000);
  server.send(200,"text/plane", thrust);
}

void sendImpulse()
{
  impulse=0;
  for (int i=0;i<1024;i++){
    if((String)arr[i]!="nan"){
      impulse=impulse+(arr[i]*0.1);
    }
  }
  server.send(200,"text/plane", (String)impulse);
}

void sendFile() 
{
  String file;
  for (int i = 0; i < count; i++){
    sec+=0.1;
    file +=(String)arr[i] + "," + sec +"," + "\n";
  }
  server.send(200,"text/csv", file);
}

void calibrate()
{
    scale.tare();
}

void clearArray() 
{
  for (int i=0;i<1024;i++)
    arr[i]=NAN;
  count=0;
  sec=0;
}

void setup(void) 
{
  clearArray();
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
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
}

void loop() 
{
  server.handleClient(); 
  if (measureState=="1")
    clearArray();
  for (int i=0; measureState == "1"&&i<1024; i++) {
    if(i==1023)
    measureState="0";
    delay(87);
    newtons = scale.get_units()* 0.035274*9,80665/1000;
    arr[i]=newtons;
    count++;
    server.handleClient();
  }
}
