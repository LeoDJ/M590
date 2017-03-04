/*
created March 2017
by Leandro Späth
*/

#ifndef M590_h //handle including library twice
#define M590_h

#include <Arduino.h>
#include <SoftwareSerial.h>

#define COMMAND_TIMEOUT 1000
#define ASYNC_TIMEOUT   10000

enum m590ResponseCode {
    M590_SUCCESS,
    M590_FAILURE,
    M590_TIMEOUT,
    M590_LENGTH_EXCEEDED,
    M590_ASYNC_RUNNING,
    M590_UNDEFINED
};

enum m590States {
    M590_STATE_SHUTDOWN,
    M590_STATE_STARTUP,
    M590_STATE_STARTUP_DONE,
    M590_STATE_PIN_REQUIRED,
    M590_STATE_PIN_ENTRY_DONE,
    M590_STATE_PIN_VALIDATION,
    M590_STATE_PIN_VALIDATION_DONE,
    M590_STATE_CELLULAR_CONNECTING,
    M590_STATE_CELLULAR_CONNECTED,
};

class M590 {
public:
    M590();

    bool begin(unsigned long baudRate = 9600,
               byte rxPin = 3,
               byte txPin = 2);

    void enableDebugSerial(HardwareSerial *debugSerial = NULL);

    int available();

    char read();

    void write(const char c);

    void print(String s);

    void initialize(String pin = "");

    void loop();

    bool checkAlive(void(*callback)(void) = NULL);

    bool checkPinRequired(void(*callback)(void) = NULL);

private:
    SoftwareSerial *_gsmSerial;
    HardwareSerial *_debugSerial;
    m590States _currentState = M590_STATE_SHUTDOWN;
    m590States _previousState = M590_STATE_SHUTDOWN;
    unsigned long _asyncStartTime = 0;

    byte asyncMatchedChars = 0;

    void sendCommandWithoutPrefix(const char *progmemCommand,
                                  const char *params = NULL);

    void sendCommand(const char *progmemCommand,
                     const char *params = NULL);

    m590ResponseCode readForAsyncResponse(const char *progmemResponseString,
                                          const unsigned int timeout = ASYNC_TIMEOUT);

    m590ResponseCode readForResponse(const char *progmemResponseString,
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

};

#endif

/* void _handleReturn(String retStr); //handle an async answer from the module
     void _parseReturn(char c);

     void _parseSerial();*/


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