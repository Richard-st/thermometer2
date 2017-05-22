#include "myEEPROM.h"
#include <ArduinoOTA.h>

#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include <Base64.h>
#include <DallasTemperature.h>

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

//-------------------------------------------
// Program Definitions
//-------------------------------------------

  // Serial Messages
  #define DEBUG
  #ifdef DEBUG
    #define DEBUG_PRINT(x)     Serial.print (x)
    #define DEBUG_PRINTLN(x)   Serial.println (x)
  #else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
  #endif

  // Debug Flasher
  #define DEBUG_LED_PIN 5
  #define DEBUG_FLASH_MAIN_LOOP 1
  #define DEBUG_FLASH_ENTER_SETUP 2
  #define DEBUG_FLASH_WIFI_CONNECT 3
  #define DEBUG_FLASH_MQTT_CONNECT 4
  #define DEBUG_FLASH_SETUP 5
  #define DEBUG_FLASH_SLEEP 10

  #define MODE_SWITCH_PIN 13
  #define MODE_WORK 0
  #define MODE_SETUP 1

  #define WEBPAGE_STATE_INIT 0
  #define WEBPAGE_STATE_NORMAL 1

  #define MQTT_TOPIC "home/poolSpa/tempSensor/poolTemp"


//-------------------------------------------
// Global Variables
//-------------------------------------------

  int               iWebPageState = 0;

  int               _iLastOTA = 0;


// MQTT Variables
  WiFiClient        espClient;
  PubSubClient      client(espClient);

  int               iMQTTConnectAttempts;

// DS10b20 definitions
  #define           ONE_WIRE_BUS 2  // DS18B20 pin
  OneWire           oneWire(ONE_WIRE_BUS);
  DallasTemperature DS18B20(&oneWire);


// Setup Webserver variable

const char *SetupSsid = "IOT_TERMOMETER_SETUP";

String sPageHtml = "<!DOCTYPE html><html lang='en'><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Thermometer Setup</title><style type='text/css'> html, body { height: 100%; margin: 0; font-family: 'Helvetica','Arial', 'Helvetica'; width:100%; } header { background-color:black; color:white; font-size: 35px; text-align:center; padding:5px; } section{ } .content-box{ height:200px; min-width:400px; width:33%; float:left; border-color:black border-collapse: collapse; border-style: solid; border-width: 1px; } .box-title{ width: 100%; font-size: 16px; font-weight: bold; float:left; text-align:center; padding:5px; margin-top:15px; margin-bottom:20px; } .box-line{ width:100%; float:left; margin-bottom:5px; } .box-text{ max-width: 150px; min-width: 150px; font-size: 14px; float:left; padding-right:10px; text-align:right; } .box-entry{ max-width: 50px; min-width: 50px; font-size: 14px; float:left; } footer { background-color:black; color:white; clear:both; text-align:center; padding:5px; } @media screen and (max-width: 400px) { .content-box{ background: white; height:180px; } .box-title{ margin-top:5px; margin-bottom:10px; } header { font-size: 25px; } } </style><!--[if IE]> <script src='http://html5shim.googlecode.com/svn/trunk/html5.js'></script> <![endif]--></head><body><header> ESP8266 Setup </header><form method='post'><section><div class='content-box'><div class='box-title'> Wifi Configuration </div><div class='box-line'><div class='box-text'> Select Wifi Network </div><div class='box-entry'>~~ssidlist</div></div><div class='box-line'><div class='box-text'> Password </div><div class='box-entry'><input type='text' name='password' value='~~password'></div></div></div><div class='content-box'><div class='box-title'> Enter MQTT Server </div><div class='box-line'><div class='box-text'> Server </div><div class='box-entry'><input type='text' name='mqtt-server' value='~~mqtt-server'></div></div><div class='box-line'><div class='box-text'> Port </div><div class='box-entry'><input type='text' name='mqtt-port' value='~~mqtt-port'><br></div></div><div class='box-line'><div class='box-text'> Sleep Time </div><div class='box-entry'><input type='text' name='sleep-time' value='~~sleep-time'><br></div></div></div><div class='content-box'><div class='box-title'> Control Settings </div><div class='box-line'><div class='box-text'> Max Wifi Attempts </div><div class='box-entry'><input type='text' name='Max-Wifi' value='~~Max-Wifi'></div></div><div class='box-line'><div class='box-text'> Max MQTT Attempts </div><div class='box-entry'><input type='text' name='Max-MQTT' value='~~Max-MQTT'></div></div><div class='box-line'><div class='box-text'> Debug Led </div><div class='box-entry'><input type='radio' name='debug-led' value='debug-on' ~~parm-debug-on > On <input type='radio' name='debug-led' value='debug-off' ~~parm-debug-off > Off </div></div></div><!-- <input type='hidden' name='submitted' value='yes'> --><!-- <br><input type='submit' value='Save Changes and Restart'> --></section><footer><button type='submit' name='formAction' value='save'> Save </button><button type='submit' name='formAction' value='exit'> Exit </button></footer></form></body></html>";

ESP8266WebServer server(80);

String sWIFINetworks;

//-------------------------------------------
// EEPROM Vars
//-------------------------------------------

 myEEPROM _myEEPROM;




// function declarations
void work_mode();

//-------------------------------------------



//-------------------------------------------
void debug_flash(int noOfFlashes) {
//-------------------------------------------
  int i;

  pinMode(DEBUG_LED_PIN, OUTPUT);

  for (i=1;i<=noOfFlashes;i++){

    digitalWrite(DEBUG_LED_PIN, HIGH);
    delay(300);
    digitalWrite(DEBUG_LED_PIN, LOW);
    delay(300);
  }
  delay(500);
}
//-------------------------------------------

//-------------------------------------------
int checkSetupMode(){
//-------------------------------------------
  //
  // detect if mode switch is requested
  //
  // if MODE_SWITCH_PIN high - continue working mode
  // if MODE_SWITCH_PIN low - go into setup mode after 3 seconds of remaining low
  //
  // returns
  //
  // MODE_WORK 0
  // MODE_SETUP 1
  //

  pinMode(MODE_SWITCH_PIN, INPUT);

  if (digitalRead(MODE_SWITCH_PIN)==HIGH) return MODE_WORK;

  // pin 13 most be low
  //
  // wait 3 seconds and try again
  //

  delay (3000);
  if (digitalRead(MODE_SWITCH_PIN)==HIGH) return MODE_WORK;

  //is MODE_SWITCH_PIN remain low for 3 seconds, go into setup mode

  debug_flash(DEBUG_FLASH_ENTER_SETUP);

  return MODE_SETUP;
}

//-------------------------------------------
// Connect to MQTT Server
//-------------------------------------------
void mqtt_connect() {

  char mqtt_server[50];


  iMQTTConnectAttempts = 1;
  DEBUG_PRINT("Attempting MQTT connection on [");
  DEBUG_PRINT(_myEEPROM.getServer().c_str());
  DEBUG_PRINT(":");
  DEBUG_PRINT(_myEEPROM.getPort());
  DEBUG_PRINTLN("] ");
  //client.setServer(_myEEPROM.getServer().c_str(),_myEEPROM.getPort());
  strcpy(mqtt_server ,_myEEPROM.getServer().c_str() );
  client.setServer(mqtt_server,_myEEPROM.getPort());

  // Loop until we're reconnected
    while (!client.connected() && iMQTTConnectAttempts < _myEEPROM.getMaxMqttAttempts()) {
      //debug_flash(DEBUG_FLASH_MQTT_CONNECT);


      // Attempt to connect
      if (client.connect("Thermometer")) {
        delay(10);
        DEBUG_PRINTLN("connected");

    } else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(client.state());
      DEBUG_PRINTLN(" try again one seconds");
      // Wait 1 seconds before retrying
      iMQTTConnectAttempts++;
      delay(1000);
    }
  }

  if (!client.connected()){
      DEBUG_PRINTLN(" Failed to establish MQTT connection");
  }

}




//-------------------------------------------
// Connect to WIFI
//-------------------------------------------
void wifi_connect() {
  int iWIFIConnectAttempts = 0;
  DEBUG_PRINTLN("Connecting to wifi");



  WiFi.begin( _myEEPROM.getSSID().c_str(), _myEEPROM.getPassword().c_str());
  while (WiFi.status() != WL_CONNECTED &&  iWIFIConnectAttempts++ < _myEEPROM.getMaxWifiAttempts()) {
    debug_flash(DEBUG_FLASH_WIFI_CONNECT);
    delay(100);
    DEBUG_PRINT(".");
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINT("Connected to ");
  DEBUG_PRINTLN(_myEEPROM.getSSID());
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
}

//-------------------------------------------
// build Web Page for setup mode
//-------------------------------------------
void getWifiList(){

  bool bSelectedMatch = false;

  //start up html page //
  sWIFINetworks = "<select  name='ssidlist'  >";

  DEBUG_PRINTLN("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  DEBUG_PRINTLN("scan done");
  if (n == 0)
     sWIFINetworks +=("no networks found");
  else
  {
    sWIFINetworks +=(n);
    sWIFINetworks += (" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      sWIFINetworks += "<option value='" ;
      sWIFINetworks +=(WiFi.SSID(i));
      sWIFINetworks += "'";
      if (String (WiFi.SSID(i)) == _myEEPROM.getSSID() )sWIFINetworks += " selected";
      sWIFINetworks += ">";
      sWIFINetworks +=(WiFi.SSID(i));
      sWIFINetworks += "</option>";

      sWIFINetworks.replace("Poppy's","Poppy&#x27s");

      delay(10);
    }
  }
  //Finish html page //
  sWIFINetworks += "</select> ";
}

//-------------------------------------------
//  process return from web page
//-------------------------------------------
void handleRoot() {

  String sSubmitted = "No";
  int iEEPROMPointer = 0;

  String sPageHtmlWorking = sPageHtml;

  DEBUG_PRINTLN("handleRoot");



  switch (iWebPageState) {

    case WEBPAGE_STATE_INIT :
         DEBUG_PRINTLN("WEBPAGE_STATE_INIT");
       //  server.send(200, "text/html", sPageHtml.c_str() );
         iWebPageState = WEBPAGE_STATE_NORMAL;
         break;

    case WEBPAGE_STATE_NORMAL :
         DEBUG_PRINTLN("WEBPAGE_STATE_NORMAL");

         if ( server.arg("formAction") == "save")

              _myEEPROM.setSSID            (server.arg("ssidlist") );
              _myEEPROM.setPassword        (server.arg("password") );
              _myEEPROM.setServer          (server.arg("mqtt-server") );
              _myEEPROM.setPort            (server.arg("mqtt-port").toInt() );
              _myEEPROM.setMaxWifiAttempts (server.arg("Max-Wifi").toInt() );
              _myEEPROM.setMaxMqttAttempts (server.arg("Max-MQTT").toInt() );
              _myEEPROM.setSleepTime       (server.arg("sleep-time").toInt() );
              if ( String(server.arg("debug-led")) == String("debug-on") )
                 _myEEPROM.setDebug        (true);
              else
                 _myEEPROM.setDebug        (false);
              _myEEPROM.burn();
              DEBUG_PRINTLN("Save Pressed");

         if ( server.arg("formAction") == "exit"){
              _myEEPROM.setSSID            (server.arg("ssidlist") );
              _myEEPROM.setPassword        (server.arg("password") );
              _myEEPROM.setServer          (server.arg("mqtt-server") );
              _myEEPROM.setPort            (server.arg("mqtt-port").toInt() );
              _myEEPROM.setMaxWifiAttempts (server.arg("Max-Wifi").toInt() );
              _myEEPROM.setMaxMqttAttempts (server.arg("Max-MQTT").toInt() );
              _myEEPROM.setSleepTime       (server.arg("sleep-time").toInt() );
              _myEEPROM.setDebug           (server.arg("debug-led").toInt() );
              _myEEPROM.burn();
              DEBUG_PRINTLN("Exit Pressed");
              work_mode();
         }
         break;

    default:
         break;

  }


  // populate Page from EEPROM

  _myEEPROM.fetch();

  getWifiList();
  sPageHtmlWorking.replace("~~ssidlist"    , sWIFINetworks.c_str());    //substitute WIFIs into html
  sPageHtmlWorking.replace("~~password"    ,_myEEPROM.getPassword() );
  sPageHtmlWorking.replace("~~mqtt-server" ,_myEEPROM.getServer() );
  sPageHtmlWorking.replace("~~mqtt-port"   ,String(_myEEPROM.getPort())  );
  sPageHtmlWorking.replace("~~Max-Wifi"    ,String(_myEEPROM.getMaxWifiAttempts()) );
  sPageHtmlWorking.replace("~~Max-MQTT"    ,String(_myEEPROM.getMaxMqttAttempts()) );
  sPageHtmlWorking.replace("~~sleep-time"  ,String(_myEEPROM.getSleepTime()) );
  if (_myEEPROM.getDebug() == true){
        sPageHtmlWorking.replace("~~parm-debug-on","checked");
        sPageHtmlWorking.replace("~~parm-debug-off","");
  }
  else{
        sPageHtmlWorking.replace("~~parm-debug-on","");
        sPageHtmlWorking.replace("~~parm-debug-off","checked");

  }

  server.send(200, "text/html", sPageHtmlWorking.c_str() );


}

//-------------------------------------------
// POST Temperature
//-------------------------------------------
 void work_mode(){

    float  temp;
    char   cTemp[30];
    String sTemp ;


    debug_flash(DEBUG_FLASH_MAIN_LOOP);

    //
    // main Startup
    //


    //
    //check wifi connection
    //
    if (WiFi.status() != WL_CONNECTED) {
      wifi_connect();
    }

    ArduinoOTA.setHostname("Thermometer");
    ArduinoOTA.setPassword("123");

    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("OTA ready");

    //
    // check for OTA update
    //
    long now = millis();
    _iLastOTA = now;

    DEBUG_PRINTLN("OTA: Listening: ");

    while (_iLastOTA - now < 10000){
      _iLastOTA = millis();
      ArduinoOTA.handle();
      delay(10);
    }
    DEBUG_PRINTLN("OTA: done Listening ");

    //
    //get temperature
    //
    do {
      DS18B20.requestTemperatures();
      temp = DS18B20.getTempCByIndex(0);
      DEBUG_PRINT("Temperature: ");
      DEBUG_PRINTLN(temp);
    } while (temp == 85.0 || temp == (-127.0));


    //
    // create MQTT Brocker connection
    //
    if (!client.connected()&& WiFi.status() == WL_CONNECTED ) {
      mqtt_connect();
    }

    //
    // Publish Temp to MQTT broket
    //
    if (client.connected()){
      sTemp = String(temp,2);
      sTemp.toCharArray(cTemp,10);
      DEBUG_PRINT("Publishing Message Topic:[");
      DEBUG_PRINT( MQTT_TOPIC );
      DEBUG_PRINT("] Payload:[");
      DEBUG_PRINT(cTemp);
      DEBUG_PRINTLN("]");
      client.publish(MQTT_TOPIC,cTemp );
      delay(10);
    }


    //
    // Sleep
    //
    int i70Mins   = 4200; // 70 minutes
    int iSleepTime = _myEEPROM.getWorkingSleepTime();
    unsigned int iSleepTimeMIS;

    if (iSleepTime > i70Mins){
      //over 70 mins. will overflow so put back to 70 mins
      iSleepTime    = iSleepTime - i70Mins;
      iSleepTimeMIS = i70Mins * 1000000;
      _myEEPROM.setWorkingSleepTime(iSleepTime);
      _myEEPROM.burn();
    }else
    {
      //under 70 mins. will not overflow
      iSleepTimeMIS = iSleepTime * 1000000;
      _myEEPROM.setWorkingSleepTime(_myEEPROM.getSleepTime() );
    }


    DEBUG_PRINT("sleeping for ");
    DEBUG_PRINT(iSleepTimeMIS);
    DEBUG_PRINTLN(" microseconds");
    DEBUG_PRINT("renamining sleep time ");
    DEBUG_PRINTLN(iSleepTime);
    DEBUG_PRINTLN("going to sleep");
    debug_flash(DEBUG_FLASH_SLEEP);
    ESP.deepSleep(iSleepTimeMIS, WAKE_RF_DEFAULT); // Sleep for 60 seconds
 }

//-------------------------------------------
// Setup Webserver to get connection parameters and system settings
//-------------------------------------------
void  setup_mode(){
  DEBUG_PRINTLN("Configuring access point...");

  WiFi.softAP(SetupSsid);

  IPAddress myIP = WiFi.softAPIP();
  DEBUG_PRINT("AP IP address: ");
  DEBUG_PRINTLN(myIP);
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");

  getWifiList();
}

//-------------------------------------------
// One Off Setup
//-------------------------------------------
void setup() {
  delay(100);

  #ifdef DEBUG
    Serial.begin(115200);
  #endif

  _myEEPROM.fetch();

  if (checkSetupMode()== MODE_WORK){
    if ( _myEEPROM.getDebug()  == 1)  // Only go into work mode if we have variables present
        DEBUG_PRINTLN("Entering Workmode ");
        work_mode();
  }else{
    DEBUG_PRINTLN("Entering Setup ");
    setup_mode();
  }

}



//-------------------------------------------
// Control Loop
//-------------------------------------------
void loop() {
  //
  // Only Used When in setup mode
  //
  server.handleClient();
}
