/*
created September 2016
by Leandro Späth
*/

#ifndef M590_h //handle including library twice
#define M590_h

#include <Arduino.h>

class M590
{
  public:
    M590(Serial gsmSerial);
    void loop(); //function called on every loop
    void begin(String pin); //connect to cellular network with pin
    byte getSignalStrength(); //return current signal strength
    void beginGPRS(String apn); //connect data link with APN
    void beginGPRS(String apn, String user, String pass); //connect data link with APN login credentials
    bool connectTCP(ip, port); //initialize TCP connection, returns success
    bool disconnectTCP(); //disconnect TCP connection
    String doRequest(String req); //perform an HTTP request
    String dns(String host); //perform a DNS request
    bool sendAT(String cmd); //send an AT command directly to the M590 module
  private:
    void handleReturn(String retStr); //handle an async answer from the module
    void parseReturn(char c);
    void parseSerial();
};

#endif
