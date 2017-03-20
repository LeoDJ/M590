/*
created March 2017
by Leandro Späth
*/

#ifndef M590_H //handle including library twice
#define M590_H

#include <Arduino.h>
#include <SoftwareSerial.h>

#include <M590_Constants.h>
#include <M590_GPRSClient.h>

#define COMMAND_TIMEOUT 1000
#define ASYNC_TIMEOUT   20000
#define STATUS_POLLING_RATE 250



class M590 {
public:
    M590();

    bool begin(unsigned long baudRate = 9600,
               byte rxPin = 3,
               byte txPin = 2);

    void enableDebugSerial(HardwareSerial *debugSerial = NULL);

    int available();

    bool initialize(String pin = "");

    void shutdown();

    void loop();

    bool checkAlive(void(*callback)(void) = NULL);

    bool checkPinRequired();

    bool sendPinEntry(String pin, void(*callback)(void) = NULL);

    m590NetworkStates checkNetworkState();

    char read();

    void write(const char c);

    void print(String s);

    bool cellularReady();

    bool waitForRegistration(const unsigned int timeout);

    m590States getCurrentState();

private:
    SoftwareSerial *_gsmSerial;
    HardwareSerial *_debugSerial;
    m590States _currentState = M590_STATE_SHUTDOWN;
    m590States _previousState = M590_STATE_SHUTDOWN;
    unsigned long _asyncStartTime = 0;
    byte _asyncBytesMatched = 0;
    byte _asyncResponseLength = 0;
    const char *_asyncProgmemResponseString = NULL;
    char _responseBuffer[16];
    byte asyncMatchedChars = 0;

    void sendCommandWithoutPrefix(const char *progmemCommand,
                                  const char *params = NULL);

    void sendCommand(const char *progmemCommand,
                     const char *params = NULL);

    void resetAsyncVariables();

    m590ResponseCode readForAsyncResponse(const char *progmemResponseString = NULL,
                                          const unsigned int timeout = ASYNC_TIMEOUT);

    //if given a buffer pointer, the buffer will contain the response data after the colon
    m590ResponseCode readForResponse(const char *progmemResponseString,
                                     char *buffer = NULL,
                                     const unsigned int max_bytes = 0,
                                     const unsigned int timeout = COMMAND_TIMEOUT);

    m590ResponseCode readForResponses(const char *progmemResponseString,
                                      const char *progmemFailString,
                                      const unsigned int timeout = COMMAND_TIMEOUT);

    m590ResponseCode serialToBuffer(char *buffer,
                                    const char readUntil,
                                    const unsigned int max_bytes,
                                    const unsigned int timeout = COMMAND_TIMEOUT);

    m590ResponseCode readUntil(const char readUntil,
                               const unsigned int timeout = COMMAND_TIMEOUT);

    bool bufferStartsWithProgmem(char *buffer,
                                 const char *progmemString);

    void printDebug(const char *progmemString, bool withNewline = true);

    void printDebug(const String s, bool withNewline = true);

};

#endif

/* void _handleReturn(String retStr); //handle an async answer from the module
     void _parseReturn(char c);

     void _parseSerial();*/

//TODO: implement some of the following functions
/*void loop(); //function called on every loop
//void begin(String pin); //connect to cellular network with pin
byte getSignalStrength(); //return current signal strength
void beginGPRS(String apn); //connect data link with APN
void beginGPRS(String apn, String user, String pass); //connect data link with APN login credentials
bool connectTCP(String ip, String port); //initialize TCP connection, returns success
bool disconnectTCP(); //disconnect TCP connection
String doRequest(String req); //perform an HTTP request
String dns(String host); //perform a DNS request
bool sendAT(String cmd); //send an AT command directly to the M590 module*/
