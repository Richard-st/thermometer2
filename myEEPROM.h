// Header
#ifndef myeeprom_h
#define myeeprom_h

#include "EEPROM.h"
#include <Arduino.h>

class myEEPROM
{
  public:
    myEEPROM();

    bool   getEEPROMVarsSet();

    void   setSSID(String sSSID);
    String getSSID();

    void   setPassword(String sPassword);
    String getPassword();

    void   setServer(String sServer);
    String getServer();

    void   setPort(int iPort);
    int    getPort();

    void   setMaxWifiAttempts(int iMaxWifiAttempts);
    int    getMaxWifiAttempts();

    void   setMaxMqttAttempts(int iMaxMqttAttempts);
    int getMaxMqttAttempts();

    void   setDebug(bool bDebug);
    bool   getDebug();

    void   setSleepTime(int iSleepTime);
    int    getSleepTime();

    void   setThermoSampleTime(int iThermoSampleTime);
    int    getThermoSampleTime();
    int    getThermoSampleTimeMS();

    void   setSpaTemp(int iSpaTemp);
    int    getSpaTemp();

    void   setThermPollTime(int iThermPollTime);
    int    getThermPollTime();
    int    getThermPollTimeMS();

    void   setOnButton(bool bOn);
    bool   getOnButton();

    void   burn();
    void   fetch();

    void   debug(String sTitle);


  private:

    bool _iEEPROMVarsSet ;

    struct structEEPROMMemMap{
      int   iAddrMemMap,iLenMemMap ;
      int   iAddrWifi,iLenWifi ;
      int   iAddrMQTT,iLenMqtt;
      int   iAddrControl,iLenControl;
    } _memMapFromEEPROM;

    struct structWifi{
      char   cSSid[33];
      char   cPassword[20];
    } _wifiFromEEPROM;

    struct structMQTT{
      char   cServer[20];
      int    iPort;
    } _mqttFromEEPROM;

    struct structControl{
      int    iMaxWifiAttempts;
      int    iMaxMqttAttempts;
      bool   bDebug;
      int    iSleepTime;
    } _controlFromEEPROM;


};
#endif
