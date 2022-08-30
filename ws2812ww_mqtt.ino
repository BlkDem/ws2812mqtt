#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>

const char* host = "strip29";
ESP8266HTTPUpdateServer httpUpdater;

#define WWPIN          13 //sonoff green led 
#define RELAYPIN       12 //sonoff basic relay
//#define PIN            2
#define PIN            0 //sonoff basic free pin
//#define NUMPIXELS      10
#define NUMPIXELS      10
#define BRIGHTNESS     50
#define DNS = "8.8.8.8"
#define mstep (NUMPIXELS/1)

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid1 = "samalet";
const char* password1 = "samalet123";
const char* ssid2 = "MN6";
const char* password2 = "AAACCEEF";
const int MODE_SIMPLE = 0;
const int MODE_ALL = 1;
const int MODE_RAINBOW = 2;
const int MODE_FLAME = 3;
const int MODE_THEATER = 4;
const int MODE_NIGHT = 5;
const int MODE_OFF = 6;

const int MODE_OFF_NVALUE = 0;  //Off   
const int MODE_NIGHT_NVALUE = 1;  //NightMode   Переименовать Удалить
const int MODE_THEATER_NVALUE = 2;  //Theater   Переименовать Удалить
const int MODE_RAINBOW_NVALUE = 3;  //Rainbow   Переименовать Удалить
const int MODE_FLAME_NVALUE = 4;  //Flame   Переименовать Удалить
const int MODE_ALL_NVALUE = 5;  //All Effects
const int MODE_SIMPLE_NVALUE = 6;  //Simple

const int MQTT_IDX = 29;

const int id_button = 0;
const int id_relay = 12;
//const int id_led = 13;
const int id_led = 4;
const int id_sens = 14;

const int idx_ledww = 14; //index led white led for domoticz
const int idx_ledws = 1000; //index ws2812 for domoticz

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "samalet"
#define WLAN_PASS       "samalet123"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "umodom.ru"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "maxim"
#define AIO_KEY         "ctdfcnjgjkm"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

String LOCALIP="192.168.0.49";
int CurrentLedMode = MODE_OFF; 
volatile boolean needUpdate = true;
volatile boolean setBreak = false;
String WWLedState = "off";
uint16_t lux = 0;
//#define LOCALIP "192.168.0.49"

//WiFiServer server(80);
ESP8266WebServer server ( 80 );
//WiFiClient client2;
Ticker blinker;



const char COLOR_FEED[] = "domoticz/out";
Adafruit_MQTT_Publish color_topic = Adafruit_MQTT_Publish(&mqtt, COLOR_FEED);
Adafruit_MQTT_Subscribe slider0 = Adafruit_MQTT_Subscribe(&mqtt, "/idx29/nvalue");
Adafruit_MQTT_Subscribe color0 = Adafruit_MQTT_Subscribe(&mqtt, "/idx29/color");


int gamma1[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };


    
int delayval = 50;
int r,g,b,j  = 0;
int rFlame = 255;
//int gFlame = 96;
//int bFlame = 12;
int gFlame = 56;
int bFlame = 2;
int colors[3] = {0, 0, 0};
bool updateColor = false;

void changeState()
{
  //digitalWrite(LED, !(digitalRead(LED)));  //Invert Current State of LED  
}

char* GetModeSValue(int index)
{
  switch (index)
  {
    case 0: {return "Simple"; break;}
    case 1: {return "All"; break;}
    case 2: {return "Rainbow"; break;}
    case 3: {return "Flame"; break;}
    case 4: {return "Theater"; break;}
    case 5: {return "NightMode"; break;}
    case 6: {return "Off"; break;}
    default: {return "none"; break;}
  }
}

void handleRoot() {
  String temp = "<html><head><meta http-equiv='refresh' content='60'/><title>Led Strip 1</title>";
  temp += "<style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; font-size:22px;}";
  temp += ".my_button {width: 300px; height: 32px; font-size:20px;}";
  temp += ".my_button1 {width: 80px; height: 32px; font-size:20px;}";
  temp += ".my_text {font-size:14px;}";
  temp += "</style></head>";

  temp += "<body>";
  //temp += "<body><h1>LED Strip 1</h1>";
  //temp += "<p><label>Lux: "+String(lux)+"</label></p>";
  temp += "<label>Current mode: "+ String(GetModeSValue(CurrentLedMode)) + "</label><br><label><p><font color='red'>R = "+String(r)+"</font></b> <b><font color='green'>G = "+String(g)+"</font></b> <b><font color='blue'>B = "+String(b)+"</font></b></label>";
  temp += "<form action='http://" + LOCALIP + "/setColor' method='GET'>";
  temp += "<label for=" + String(r) + ">R (0-255): </label><input type='text' name='rVal' value='"+String(r)+"'/ class='my_button1'><br />";
  temp += "<label for=" + String(g) + ">G (0-255): </label><input type='text' name='gVal' value='"+String(g)+"'/ class='my_button1'><br />";
  temp += "<label for=" + String(b) + ">B (0-255): </label><input type='text' name='bVal' value='"+String(b)+"'/ class='my_button1'><br />";
  temp += "<input type=\"submit\" value=\"Set Color\" /class='my_button'></form>";

  temp += "<form action='http://" + LOCALIP + "/setMode?ledmode=" + String(MODE_ALL) + " method='get'>";
  temp += "<input name='ledmode' type='hidden' value='" + String(MODE_ALL) + "'>";
  temp += "<input name='sub2' type='submit' value='All Effects' class='my_button'></form>";

  temp += "<form action='http://" + LOCALIP + "/setMode?ledmode=" + String(MODE_RAINBOW) + " method='get'>";
  temp += "<input name='ledmode' type='hidden' value='" + String(MODE_RAINBOW) + "'>";
  temp += "<input name='sub3' type='submit' value='Rainbow' class='my_button'></form>";

  temp += "<form action='http://" + LOCALIP + "/setMode?ledmode=" + String(MODE_THEATER) + " method='get'>";
  temp += "<input name='ledmode' type='hidden' value='" + String(MODE_THEATER) + "'>";
  temp += "<input name='sub3' type='submit' value='Theater' class='my_button'></form>";

  temp += "<form action='http://" + LOCALIP + "/setMode?ledmode=" + String(MODE_FLAME) + " method='get'>";
  temp += "<input name='ledmode' type='hidden' value='" + String(MODE_FLAME) + "'>";
  temp += "<input name='sub4' type='submit' value='Flame' class='my_button'></form>";

  temp += "<form action='http://" + LOCALIP + "/setMode?ledmode=" + String(MODE_NIGHT) + " method='get'>";
  temp += "<input name='ledmode' type='hidden' value='" + String(MODE_NIGHT) + "'>";
  temp += "<input name='sub6' type='submit' value='Night Mode' class='my_button'></form>";

  temp += "<form action='http://" + LOCALIP + "/setMode?ledmode=" + String(MODE_OFF) + " method='get'>";
  temp += "<input name='ledmode' type='hidden' value='" + String(MODE_OFF) + "'>";
  temp += "<input name='sub5' type='submit' value='Off' class='my_button'></form>";

  temp += "<label>WhiteLed State = " + WWLedState + "</label>";
  temp += "<form action='http://" + LOCALIP + "/setww?wwmode=on method='get'>";
  temp += "<input name='wwmode' type='hidden' value='on'>";
  temp += "<input name='sub7' type='submit' value='Turn On White' class='my_button'></form>";

  temp += "<form action='http://" + LOCALIP + "/setww?wwmode=off method='get'>";
  temp += "<input name='wwmode' type='hidden' value='off'>";
  temp += "<input name='sub8' type='submit' value='Turn Off White' class='my_button'></form>";

  temp += "</body></html>";

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  String sTime = "";
  String sSec = String(sec%60);
  String sMin = String(min%60);
  String sHour = String(hr);
  sTime = "HMS: " + sHour + ":" + sMin + ":" + sSec;
  temp += "<p class='my_text'><label>Uptime: "+sTime+"</label></p>";
  temp += "</body></html>";
  
  server.send ( 200, "text/html", temp );
}

void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  for (int i = 0; i<3; i++) 
  {
    ret = mqtt.connect();
    if (ret == 0) 
    {
      Serial.println("MQTT connected");
      return;
    }
    else
    { // connect will return 0 for connected
      switch (ret) 
      {
        case 1: Serial.println("Wrong protocol"); break;
        case 2: Serial.println("ID rejected"); break;
        case 3: Serial.println("Server unavailable"); break;
        case 4: Serial.println("Bad user/password"); break;
        case 5: Serial.println("Not authenticated"); break;
        case 6: Serial.println("Failed to subscribe"); break;
        default: Serial.print("Couldn't connect to server, code: ");
        Serial.println(ret);
        break;
      }
    }
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(1000);// wait 5 seconds
  }  
}

void SendDomMess(int idx, int nval, char* sval)
{
  //MQTT_connect();

        StaticJsonBuffer<300> JSONbuffer;
        JsonObject& JSONencoder = JSONbuffer.createObject();
        JSONencoder["idx"] = idx;
        JSONencoder["nvalue"] = nval;
        JSONencoder["svalue1"] = sval;
        char JSONmessageBuffer[100];
        JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        if (color_topic.publish(JSONmessageBuffer) == true) 
        {   
            //Serial.println("Success sending message");    
        } 
        else 
        {  
            //Serial.println("Error sending message");   
        }
     
//    } //now - lastMsg
}

void handleChangeWW()
{
  //WWLedState
/*  String out = "{ 'status' = 'error' }"; // Fall-backs on error by default
  char buff[11];
  if(server.method() == HTTP_GET) {
    if(isArgSet("wwmode")) 
    {
        server.arg(argPos("wwmode")).toCharArray(buff,10);
        WWLedState = String(buff);
        if (WWLedState=="on")
        {
          digitalWrite(WWPIN, LOW);
          digitalWrite(RELAYPIN, HIGH);
          SendDomMess(idx_ledww, 1, "on");
       }
        else
        {
          digitalWrite(WWPIN, HIGH);
          digitalWrite(RELAYPIN, LOW);
          SendDomMess(idx_ledww, 0, "off");
        }
        setBreak = true;
        out = "{ 'status' = 'wwmode: "+ String(buff) +"'}";
    }
    else
    {
      out = "{ 'status' = 'param wrong'" + String(server.arg(0)) + "}";
    }
  }
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");*/
}

void handleChangeColor() {
  String out = "{ 'status' = 'error' }"; // Fall-backs on error by default
  char buff[11];
  if((server.method() == HTTP_GET) && (server.args()==3) && (CurrentLedMode != MODE_SIMPLE)) {
    if(
      isArgSet("rVal") &&
      isArgSet("gVal") &&
      isArgSet("bVal")
      ) {
        server.arg(argPos("rVal")).toCharArray(buff,10);
        colors[0] = atoi(buff);
        r = colors[0];
        server.arg(argPos("gVal")).toCharArray(buff,10);
        colors[1] = atoi(buff);
        g = colors[1];
        server.arg(argPos("bVal")).toCharArray(buff,10);
        colors[2] = atoi(buff);
        b = colors[2];
        updateColor = true;
        
        CurrentLedMode=MODE_SIMPLE;   
        SendDomMess(MQTT_IDX, MODE_SIMPLE_NVALUE, "Simple");
        //SendSwitchMess(MQTT_IDX, MODE_SIMPLE_NVALUE);
        needUpdate = true;     
        setBreak = true;
        out = "{ 'status' = 'ok' }";
    }
  }
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

int GetModeNValue(int idx)
{
  switch (idx)
  {
    case 0: {return MODE_SIMPLE_NVALUE; break;}
    case 1: {return MODE_ALL_NVALUE; break;}
    case 2: {return MODE_RAINBOW_NVALUE; break;}
    case 3: {return MODE_FLAME_NVALUE; break;}
    case 4: {return MODE_THEATER_NVALUE; break;}
    case 5: {return MODE_NIGHT_NVALUE; break;}
    case 6: {return MODE_OFF_NVALUE; break;}
    default: {return MODE_OFF_NVALUE; break;}
  }
}

void UpdateMode(int cMode)
{
    updateColor = true;
        needUpdate = true;
        CurrentLedMode = cMode;
        char* sval = GetModeSValue(CurrentLedMode);
        //SendDomMess(idx_ledws, GetModeNValue(CurrentLedMode), sval);
        SendDomMess(MQTT_IDX, GetModeNValue(CurrentLedMode), sval);
        //SendSwitchMess(MQTT_IDX, GetModeNValue(CurrentLedMode));
        setBreak = true;
        
}

void handleSetMode() 
{
  String out = "{ 'status' = 'error' }"; // Fall-backs on error by default
  char buff[11];
  
  if(server.method() == HTTP_GET) {
    if(isArgSet("ledmode")) 
    {
        server.arg(argPos("ledmode")).toCharArray(buff,10);
        if (CurrentLedMode==atoi(buff))
        {
          return;
        }
       UpdateMode(atoi(buff));
       out = "{ 'status' = 'ledmode: "+ String(buff) +"'}";
    }
    else
    {
      out = "{ 'status' = 'param wrong'" + String(server.arg(0)) + "}";
    }
  }
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  /*server.send ( 200, "application/json", out);
  Serial.println("Client served");*/
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

bool isArgSet(String varname) {
  bool existsValue = false;
  for(int i=0;i<server.args();i++) {
    existsValue |= (server.argName(i) == varname);
  }
  return existsValue;
}

int8_t argPos(String varname) {
  int8_t arg_pos = -1;
  for(int i=0;i<server.args();i++) {
    if(server.argName(i)==varname) {
      arg_pos = i;
      break;
    }
  }
  return arg_pos;
}

boolean CheckWifi()
{
  return (WiFi.status() == WL_CONNECTED) ;
}

void StartWiFi()
{
  boolean sok = false;
    
  WiFi.begin(ssid1, password1);
  for (int i=0; i<20; i++)
  {   
    if (CheckWifi())
    {
      sok = true;      
      return;
    }
    digitalWrite(WWPIN, LOW);
    delay(250);
    digitalWrite(WWPIN, HIGH);
    delay(250);
    //Serial.print(".");
  }
  if (!sok)
  {
 
  for (int i=0; i<20; i++)
  {
    WiFi.begin(ssid2, password2);
    if (CheckWifi())
    {
      sok = true;
      return;
    }
    digitalWrite(WWPIN, LOW);
    delay(250);
    digitalWrite(WWPIN, HIGH);
    delay(250);
    //Serial.print(".");
  }
  }
  Serial.println("");
  Serial.println("Cannot WiFi connect");
  if ( MDNS.begin ( "ledstrip1" ) ) {
    Serial.println ( "MDNS responder started" );
  }
}

void StartServers()
{
  
  StartWiFi();
  if (!CheckWifi())
  {
    Serial.println("");
    Serial.print("No network connection");
    return;
  }
  // Starting the web server
 
  Serial.println("Web server running. Waiting for the ESP IP...");
  
  delay(500);
  
  
  // Printing the ESP IP address
  IPAddress ip = WiFi.localIP();
  //sprintf(lcdBuffer, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], udpPort);
  LOCALIP = String(ip[0])+ '.' + String(ip[1])+ '.' + String(ip[2])+ '.' + String(ip[3]);
  Serial.println(LOCALIP);
   
  server.on ( "/", handleRoot );
  server.on ( "/setColor", handleChangeColor);
  server.on ( "/setMode", handleSetMode);
  server.on ( "/setww", handleChangeWW);
  server.onNotFound ( handleNotFound );
  
  httpUpdater.setup(&server);
  server.begin();
  Serial.println ( "HTTP server started" );
  MDNS.begin ( "ledstrip1" );
  MDNS.addService("http", "tcp", 80);
}

void slidercallback(double x) {
  Serial.println(x);
  needUpdate = true;
  setBreak = true;
  UpdateMode((int)x);
}

/*void colorConverter(String hexValue)
{
  unsigned long rgb = 0x6f56a3;
  red = r >> 16 ;
  green = (rgb & 0x00ff00) >> 8;
  blue = (rgb & 0x0000ff);
  rgb = 0;
  rgb |= red <<16;
  rgb |= blue <<8;
  rgb |= green;
  Serial.println(red);
  Serial.println(green);
  Serial.println(blue);
}*/

void SetRGB(int cRGB)
{
  r = (cRGB >> 16) & 0xFF;
  Serial.print("R: ");
  Serial.println(r);
  g = (cRGB >> 8) & 0xFF;
  Serial.print("G: ");
  Serial.println(g);
  b = cRGB & 0xFF;
  Serial.print("B: ");
  Serial.println(b);
}

void colorcallback(double x) {
  Serial.print("color is: ");
  Serial.println(x);
  SetRGB((int)x);
  needUpdate = true;
  setBreak = true;
  updateColor = true;        
  CurrentLedMode=MODE_SIMPLE;   
  SendDomMess(MQTT_IDX, MODE_SIMPLE_NVALUE, "Simple");
}

void setup() {
  //blinker.attach(0.5, changeState);
  strip.begin(); // This initializes the NeoPixel library.
  pinMode(WWPIN, OUTPUT);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(WWPIN, LOW);
  Serial.begin(115200);
  delay(10);
  r = 128;
  g = 128;
  b = 128;
  StartServers();
  slider0.setCallback(slidercallback); 
  color0.setCallback(colorcallback);
  mqtt.subscribe(&slider0);
  mqtt.subscribe(&color0);
  ESP.wdtEnable(WDTO_4S);
}

void loop() {
  /*MQTT_connect();
  mqtt.processPackets(10);*/
  /*if(! mqtt.ping()) {
    mqtt.disconnect();
  }*/
  ESP.wdtFeed();
  server.handleClient();
  
  if ((CurrentLedMode==MODE_SIMPLE) && (needUpdate))
  {
    needUpdate = false;
    //uint16_t _step = 255/(NUMPIXELS+1);
    for(uint16_t i=0;i<mstep;i++)
    {
      strip.setPixelColor(mstep - i, r, g, b); 
      strip.setPixelColor(mstep + i, r, g, b);
      strip.show();
    }
    strip.setPixelColor(0, r, g, b);
    strip.show();
    /*for(uint16_t i2=1;i2<6;i2++)
    {
      server.handleClient();
      strip.setBrightness(40*i2);
      strip.show(); // This sends the updated pixel color to the hardware.
      //delay(delayval); // Delay for a period of time (in milliseconds).
    }*/
    //strip.setBrightness(255);
    //strip.show();
  }

  if ((CurrentLedMode==MODE_ALL) || (CurrentLedMode==MODE_RAINBOW))
  {
    server.handleClient();
    Serial.println("rainbowCycle");
    rainbowCycle(2);
  }
  
  if (CurrentLedMode==MODE_ALL)
  {
    server.handleClient();
    Serial.println("colorWipe");
    colorWipe(strip.Color(0, 0, 0), 50);    // Black/off
    colorWipe(strip.Color(255, 0, 0), 50);  // Red
    colorWipe(strip.Color(0, 255, 0), 50);  // Green
    colorWipe(strip.Color(0, 0, 255), 50);  // Blue
  }

  if ((CurrentLedMode==MODE_ALL) || (CurrentLedMode==MODE_THEATER))
  {
   server.handleClient();
    Serial.println("theaterChase");
    theaterChase(strip.Color(127, 127, 127), 50); // White
    mqtt.processPackets(10); ESP.wdtFeed();
    theaterChase(strip.Color(127,   0,   0), 50); // Red
    mqtt.processPackets(10); ESP.wdtFeed();
    theaterChase(strip.Color(  0,   0, 127), 50); // Blue
    mqtt.processPackets(10); ESP.wdtFeed();
  }

  if ((CurrentLedMode==MODE_ALL) || (CurrentLedMode==MODE_RAINBOW))
  {
   server.handleClient();
    Serial.println("rainbow");
    rainbow(20);
  }

  
  if ((CurrentLedMode==MODE_ALL) || (CurrentLedMode==MODE_RAINBOW))
  {
    server.handleClient();
    Serial.println("theaterChaseRainbow");
    theaterChaseRainbow(50);
  }
  
  if ((CurrentLedMode==MODE_ALL) || (CurrentLedMode==MODE_RAINBOW))
  {
    server.handleClient();
    Serial.println("whiteOverRainbow");
    whiteOverRainbow(20,75,8);  
  }

  if (CurrentLedMode==MODE_ALL)
  {
    server.handleClient();
    Serial.println("pulseWhite");
    pulseWhite(5); 
  }

  if (CurrentLedMode==MODE_ALL)
  {
    server.handleClient();
    Serial.println("fullWhite");
    fullWhite();
    delay(100);
  }
  
  if ((CurrentLedMode==MODE_ALL) || (CurrentLedMode==MODE_RAINBOW))
  {
    server.handleClient();
    Serial.println("rainbowFade2White");
    rainbowFade2White(3,3,1);
  }
  
  if ((CurrentLedMode==MODE_ALL) || (CurrentLedMode==MODE_FLAME))
  {
   server.handleClient();
    Serial.println("Flame");
    Flame();
  }

  if ((CurrentLedMode==MODE_OFF) && (needUpdate))
  { 
    needUpdate = false;
    for(uint16_t i=0; i<(mstep+1); i++) 
    {
        server.handleClient();
        //delay(20);
        strip.setBrightness(255-(255/(mstep+1)*(i+1)));
        strip.show();
    }
    strip.setBrightness(0);
    strip.show();
    strip.setBrightness(255);  
    ESP.wdtFeed();
  }

if ((CurrentLedMode==MODE_NIGHT) && (needUpdate))
  { 
    needUpdate = false;    
    //int mstep = 129;
    for(uint16_t i1=0; i1<mstep; i1++) 
    {
      for (uint16_t j=0; j<(mstep+1); j++)
      {
        strip.setPixelColor(mstep + i1, (255/(mstep+1))*(j+1),(80/(mstep+1))*(j+1),(10/(mstep+1))*(j+1) );
        strip.setPixelColor(mstep - i1, (255/(mstep+1))*(j+1),(80/(mstep+1))*(j+1),(10/(mstep+1))*(j+1) );
        //strip.setPixelColor((mstep + i1), strip.Color(250,80,10));
        //strip.setPixelColor((mstep - i1), strip.Color(250,80,10));
        
        
        server.handleClient();
        //delay(5);
        if ((j%4)==0)
        {
          
          strip.setBrightness((NUMPIXELS/mstep)*i1);
        }
        strip.show();
        ESP.wdtFeed();
      }
    }
    strip.setBrightness(255);
    strip.show();
    ESP.wdtFeed();
  }

}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


void Flame()
{
  for (int y = 0; y<300; y++)
  {
    server.handleClient();
    mqtt.processPackets(10);
    ESP.wdtFeed();
    for(int x = 0; x <strip.numPixels(); x++)
    {
      int flicker = random(0,40);
      int r1 = rFlame-flicker;
      int g1 = gFlame-flicker;
      int b1 = bFlame-flicker;
      if(g1<0) g1=0;
      if(r1<0) r1=0;
      if(b1<0) b1=0;
      strip.setPixelColor(x,r1,g1, b1);
     server.handleClient();
    }
      if (setBreak)
      {
        setBreak= false;
        break;
      }
    strip.show();
    delay(random(50,150));
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    server.handleClient();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    mqtt.processPackets(10);
    ESP.wdtFeed();
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
      server.handleClient();
      if (setBreak)
      {
        setBreak= false;
        return;
      }
    }
      
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    mqtt.processPackets(10); ESP.wdtFeed();
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      server.handleClient();
      if (setBreak)
      {
        setBreak= false;
        return;
      }
    }
    
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      mqtt.processPackets(10); ESP.wdtFeed();
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
        server.handleClient();
        if (setBreak)
        {
          setBreak= false;
          return;
        }
      }
      
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
        server.handleClient();
        if (setBreak)
        {
          setBreak= false;
          return;
        }
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel    
    for (int q=0; q < 3; q++) {
      mqtt.processPackets(10); ESP.wdtFeed();
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        server.handleClient();
        if (setBreak)
    {
      setBreak= false;
      return;
    }
      }
      strip.show();
      server.handleClient();
      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
        server.handleClient();
        if (setBreak)
    {
      setBreak= false;
      return;
    }
      }
    }
    

  }
}

void pulseWhite(uint8_t wait) {
  for(int j = 0; j < 256 ; j++){
    mqtt.processPackets(10); ESP.wdtFeed();
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma1[j] ) );
          server.handleClient();
          if (setBreak)
          {
            setBreak= false;
            return;
          }
        }
        
        delay(wait);
        strip.show();
      }

  for(int j = 255; j >= 0 ; j--){
    mqtt.processPackets(10); ESP.wdtFeed();
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma1[j] ) );
          server.handleClient();
        }
        if (setBreak)
        {
          setBreak= false;
          return;
        }
        delay(wait);
        strip.show();
      }
}


void rainbowFade2White(uint8_t wait, int rainbowLoops, int whiteLoops) {
  float fadeMax = 100.0;
  int fadeVal = 0;
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for(int k = 0 ; k < rainbowLoops ; k ++){
    
    for(int j=0; j<256; j++) { // 5 cycles of all colors on wheel
      mqtt.processPackets(10); ESP.wdtFeed();
      for(int i=0; i< strip.numPixels(); i++) {

        wheelVal = Wheel(((i * 256 / strip.numPixels()) + j) & 255);

        redVal = red(wheelVal) * float(fadeVal/fadeMax);
        greenVal = green(wheelVal) * float(fadeVal/fadeMax);
        blueVal = blue(wheelVal) * float(fadeVal/fadeMax);

        strip.setPixelColor( i, strip.Color( redVal, greenVal, blueVal ) );
        server.handleClient();
        if (setBreak)
        {
          setBreak= false;
          return;
        }

      }

      //First loop, fade in!
      if(k == 0 && fadeVal < fadeMax-1) {
          fadeVal++;
      }

      //Last loop, fade out!
      else if(k == rainbowLoops - 1 && j > 255 - fadeMax ){
          fadeVal--;
      }
        if (setBreak)
        {
          setBreak= false;
          break;
        }

        strip.show();
        delay(wait);
    }
  
  }



  delay(50);


  for(int k = 0 ; k < whiteLoops ; k ++){

    for(int j = 0; j < 256 ; j++){
        mqtt.processPackets(10); ESP.wdtFeed(); 
        for(uint16_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(0,0,0, gamma1[j] ) );
            server.handleClient();
          }
          if (setBreak)
          {
            setBreak= false;
            return;
          }
          strip.show();
        }

        delay(2000);
    for(int j = 255; j >= 0 ; j--){

        for(uint16_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(0,0,0, gamma1[j] ) );
            server.handleClient();
          }
          if (setBreak)
          {
            setBreak= false;
            return;
          }
        strip.show();
        }
  }

  delay(delayval);


}

void whiteOverRainbow(uint8_t wait, uint8_t whiteSpeed, uint8_t whiteLength ) {
  
  if(whiteLength >= strip.numPixels()) whiteLength = strip.numPixels() - 1;

  int head = whiteLength - 1;
  int tail = 0;

  int loops = 3;
  int loopNum = 0;

  static unsigned long lastTime = 0;


  while(true){
    for(int j=0; j<256; j++) {
      mqtt.processPackets(10); ESP.wdtFeed();
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        if((i >= tail && i <= head) || (tail > head && i >= tail) || (tail > head && i <= head) ){
          strip.setPixelColor(i, strip.Color(0,0,0, 255 ) );
          server.handleClient();
        }
        else{
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
          server.handleClient();
        }
        if (setBreak)
        {
          setBreak= false;
          return;
        }

      }

      if(millis() - lastTime > whiteSpeed) {
        head++;
        tail++;
        if(head == strip.numPixels()){
          loopNum++;
        }
        lastTime = millis();
      }

      if(loopNum == loops) return;
    
      head%=strip.numPixels();
      tail%=strip.numPixels();
        strip.show();
        delay(wait);
    }
  }
  
}
void fullWhite() {
  
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0,0,0, 255 ) );
        server.handleClient();
    }
    strip.show();  
}


uint8_t red(uint32_t c) {
  return (c >> 8);
}
uint8_t green(uint32_t c) {
  return (c >> 16);
}
uint8_t blue(uint32_t c) {
  return (c);
}


