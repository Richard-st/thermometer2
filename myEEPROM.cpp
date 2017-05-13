#include "myEEPROM.h"
#include <Arduino.h>


//-------------------------------
// constructor
//-------------------------------
myEEPROM::myEEPROM(){

}


//-------------------------------
// _iEEPROMVarsSet
//-------------------------------
bool   myEEPROM::getEEPROMVarsSet(){
  return _iEEPROMVarsSet;
  }


//-------------------------------
// SSID
//-------------------------------
void   myEEPROM::setSSID(String sSSID){
  strcpy(_wifiFromEEPROM.cSSid,sSSID.c_str());}

String myEEPROM::getSSID(){
  return String(_wifiFromEEPROM.cSSid);}

//-------------------------------
// Password
//-------------------------------
void   myEEPROM::setPassword(String sPassword){
  strcpy(_wifiFromEEPROM.cPassword,sPassword.c_str());}

String myEEPROM::getPassword(){
  return String(_wifiFromEEPROM.cPassword);}

//-------------------------------
// Server
//-------------------------------
void   myEEPROM::setServer(String sServer){
  strcpy(_mqttFromEEPROM.cServer,sServer.c_str());}

String myEEPROM::getServer(){
  return String(_mqttFromEEPROM.cServer);}

//-------------------------------
// Port
//-------------------------------
void   myEEPROM::setPort(int iPort){
  _mqttFromEEPROM.iPort = iPort;}

int  myEEPROM::getPort(){
  return _mqttFromEEPROM.iPort;}

//-------------------------------
// MaxWifiAttempts
//-------------------------------
void   myEEPROM::setMaxWifiAttempts(int iMaxWifiAttempts){
  _controlFromEEPROM.iMaxWifiAttempts = iMaxWifiAttempts;}

int  myEEPROM::getMaxWifiAttempts(){
  return _controlFromEEPROM.iMaxWifiAttempts;}

//-------------------------------
// MaxMqttAttempts
//-------------------------------
void   myEEPROM::setMaxMqttAttempts(int iMaxMqttAttempts){
  _controlFromEEPROM.iMaxMqttAttempts = iMaxMqttAttempts;}

int  myEEPROM::getMaxMqttAttempts(){
  return _controlFromEEPROM.iMaxMqttAttempts;}

//-------------------------------
// Debug
//-------------------------------
void   myEEPROM::setDebug(bool bDebug){
  _controlFromEEPROM.bDebug = bDebug;}

bool  myEEPROM::getDebug(){
  return _controlFromEEPROM.bDebug;}

//--------------------------------------------------
// sleep Time
//--------------------------------------------------

void    myEEPROM::setSleepTime(int iSleepTime){
  _controlFromEEPROM.iSleepTime = iSleepTime;}

int     myEEPROM::getSleepTime(){
  return _controlFromEEPROM.iSleepTime;}


//-------------------------------
// burn EEPROM
//-------------------------------
void myEEPROM::burn()
{
  //setup Memmap

  _memMapFromEEPROM.iAddrMemMap  = 1; //position zero is used for data present flag
  _memMapFromEEPROM.iLenMemMap   = sizeof(_memMapFromEEPROM);

  _memMapFromEEPROM.iAddrWifi    = _memMapFromEEPROM.iAddrMemMap + _memMapFromEEPROM.iLenMemMap;
  _memMapFromEEPROM.iLenWifi     = sizeof(_wifiFromEEPROM);

  _memMapFromEEPROM.iAddrMQTT    = _memMapFromEEPROM.iAddrWifi + _memMapFromEEPROM.iLenWifi;
  _memMapFromEEPROM.iLenMqtt     = sizeof(_mqttFromEEPROM);

  _memMapFromEEPROM.iAddrControl = _memMapFromEEPROM.iAddrMQTT + _memMapFromEEPROM.iLenMqtt;
  _memMapFromEEPROM.iLenControl  = sizeof(_controlFromEEPROM);

  EEPROM.begin(512);
  debug("  EEPROM BEFORE WRITE");

  EEPROM.put(0                              , true);
  EEPROM.put(1                              , _memMapFromEEPROM);
  EEPROM.put(_memMapFromEEPROM.iAddrWifi    , _wifiFromEEPROM);
  EEPROM.put(_memMapFromEEPROM.iAddrMQTT    , _mqttFromEEPROM);
  EEPROM.put(_memMapFromEEPROM.iAddrControl , _controlFromEEPROM);
  EEPROM.end();
}

//-------------------------------
// burn EEPROM
//-------------------------------
void myEEPROM::fetch()
{
  EEPROM.begin(512);
  EEPROM.get(0                              , _iEEPROMVarsSet);
  EEPROM.get(1                              , _memMapFromEEPROM);
  EEPROM.get(_memMapFromEEPROM.iAddrWifi    , _wifiFromEEPROM);
  EEPROM.get(_memMapFromEEPROM.iAddrMQTT    , _mqttFromEEPROM);
  EEPROM.get(_memMapFromEEPROM.iAddrControl , _controlFromEEPROM);
  EEPROM.end();
  debug("  EEPROM AFTER READ");

}

//-------------------------------
// debug Structures
//-------------------------------
void myEEPROM::debug(String sTitle)
{
   Serial.println("=============================");
   Serial.println(sTitle);
   Serial.println("=============================");
   Serial.print  (String ("== _iEEPROMVarsSet  - ")) ;
   Serial.println(String(_iEEPROMVarsSet) );
   Serial.print  (String ("== cSSid            - ")) ;
   Serial.println(String(_wifiFromEEPROM.cSSid) );
   Serial.print  (String ("== cPassword        - ")) ;
   Serial.println(String(_wifiFromEEPROM.cPassword) );
   Serial.print  (String ("== cServer          - ")) ;
   Serial.println(String(_mqttFromEEPROM.cServer) );
   Serial.print  (String ("== iPort            - ")) ;
   Serial.println(String(_mqttFromEEPROM.iPort) );
   Serial.print  (String ("== iSleepTime       - ")) ;
   Serial.println(String(_controlFromEEPROM.iSleepTime) );
   Serial.print  (String ("== iMaxWifiAttempts - ")) ;
   Serial.println(String(_controlFromEEPROM.iMaxWifiAttempts) );
   Serial.print  (String ("== iMaxMqttAttempts - ")) ;
   Serial.println(String(_controlFromEEPROM.iMaxMqttAttempts) );
   Serial.print  (String ("== bDebug           - ")) ;
   Serial.println(String(_controlFromEEPROM.bDebug) );

   return;
}
