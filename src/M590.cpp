/*
created March 2017
by Leandro Späth
*/

#include <Arduino.h>
#include <M590.h>

const char
        M590_COMMAND_GET_SIM_IDENTIFICATION[]   PROGMEM = "CCID",
        M590_COMMAND_CHECK_STATUS[]             PROGMEM = "CPAS",
        M590_COMMAND_CHECK_NETWORK_STATUS[]     PROGMEM = "CREG?",
        M590_COMMAND_CHECK_PIN[]                PROGMEM = "CPIN?",
        M590_COMMAND_INPUT_PIN[]                PROGMEM = "CPIN=",
        M590_COMMAND_SHUTDOWN[]                 PROGMEM = "CPWROFF",
        M590_COMMAND_GET_SIGNAL_STRENGTH[]      PROGMEM = "CSQ",
        M590_COMMAND_GET_NATIVE_NUMBER[]        PROGMEM = "CNUM";

const char
        M590_RESPONSE_PREFIX[]       PROGMEM = "+",
        M590_RESPONSE_SEPERATOR[]    PROGMEM = ": ",
        M590_RESPONSE_OK[]           PROGMEM = "OK\r\n",
        M590_RESPONSE_ERROR[]        PROGMEM = "ERROR\r\n",
        M590_RESPONSE_FAIL[]         PROGMEM = "FAIL\r\n";

const char
        M590_AT[]                    PROGMEM = "AT",
        M590_CRLF[]                  PROGMEM = "\r\n",
        M590_COMMAND_PREFIX[]        PROGMEM = "AT+",
        M590_CONTENT_LENGTH_HEADER[] PROGMEM = "Content-Length: ";

const char
        M590_ERROR_NOT_RESPONDING[] PROGMEM = "The M590 did not respond to an \"AT\". Please check serial connection, power supply and ONOFF pin.";


const char
        M590_LOG_01[]   PROGMEM = "Shutdown",
        M590_LOG_02[]   PROGMEM = "In Startup",
        M590_LOG_03[]   PROGMEM = "Module is active.",
        M590_LOG_04[]   PROGMEM = "Pin entry is required",
        M590_LOG_05[]   PROGMEM = "Pin entry successful",
        M590_LOG_06[]   PROGMEM = "Pin is being validated",
        M590_LOG_07[]   PROGMEM = "Pin validation successful",
        M590_LOG_08[]   PROGMEM = "Registering on cellular network",
        M590_LOG_09[]   PROGMEM = "Connected to cellular network";

//crude method of accessing multiple progmem Strings easily
const char *M590_LOG[] = {
        M590_LOG_01,
        M590_LOG_02,
        M590_LOG_03,
        M590_LOG_04,
        M590_LOG_05,
        M590_LOG_06,
        M590_LOG_07,
        M590_LOG_08,
        M590_LOG_09,
};


M590::M590() {
    _gsmSerial = NULL;
}

bool M590::begin(unsigned long baudRate, byte rxPin, byte txPin) {
    if (!_gsmSerial) {
        _gsmSerial = new SoftwareSerial(rxPin, txPin);
    }

    _gsmSerial->begin(baudRate);
}

void M590::enableDebugSerial(HardwareSerial *debugSerial) {
    if (debugSerial)
        _debugSerial = debugSerial;
}

// Serial passthrough operations
int M590::available() {
    return _gsmSerial->available();
}

char M590::read() {
    return (char) _gsmSerial->read();
}

void M590::write(const char c) {
    _gsmSerial->write(c);
}

void M590::print(const String s) {
    _gsmSerial->print(s);
}

void M590::initialize(String pin) {
    if (!checkAlive() && _debugSerial) //checkAlive still gets executed
        _debugSerial->println((__FlashStringHelper *) M590_ERROR_NOT_RESPONDING);

}

//thee loop gets called every arduino code to handle "async" responses
void M590::loop() {
    switch (_currentState) {
        case M590_STATE_STARTUP_DONE:
            break;

        case M590_STATE_PIN_VALIDATION:
            if (_gsmSerial->available()) {
                //is byte +

            }
            break;
    }
    if (_debugSerial && _previousState != _currentState) {
        _debugSerial->print((__FlashStringHelper *) M590_LOG[_currentState]);
    }
    _previousState = _currentState;
}

bool M590::checkAlive(void(*callback)(void)) {
    if (_currentState == M590_STATE_SHUTDOWN) {
        sendCommandWithoutPrefix(M590_AT);
        if (readForResponse(M590_RESPONSE_OK) == M590_SUCCESS) {
            _currentState = M590_STATE_STARTUP_DONE;
            if (callback) callback();
            return true;
        } else
            return false;
    } else return false;
}

bool M590::checkPinRequired(void (*callback)(void)) {
    if(_currentState == M590_STATE_STARTUP_DONE) {
        sendCommand(M590_COMMAND_CHECK_PIN);

    }
    else return false;
}


void M590::sendCommand(const char *progmemCommand, const char *params) {
    //need to cast to FlashStringHelper for it to correctly read from progmem
    _gsmSerial->print((__FlashStringHelper *) M590_COMMAND_PREFIX);
    sendCommandWithoutPrefix(progmemCommand, params);
}

void M590::sendCommandWithoutPrefix(const char *progmemCommand, const char *params) {
    _gsmSerial->print((__FlashStringHelper *) progmemCommand);

    if (params && strlen(params))
        _gsmSerial->print(params);

    _gsmSerial->println(); //terminate with CLRF
}

m590ResponseCode M590::readForAsyncResponse(const char *progmemResponseString, const unsigned int timeout) {
    if (_asyncStartTime == 0)
        _asyncStartTime = millis();
    if (millis() > _asyncStartTime + timeout)
        return M590_TIMEOUT;

}

m590ResponseCode M590::readForResponse(const char *progmemResponseString, const unsigned int timeout) {
    byte matched = 0;
    byte responseLength = strlen_P(progmemResponseString);
    unsigned long startTime = millis();

    while (millis() < (startTime + timeout)) {
        if (_gsmSerial->available()) {
            if (_gsmSerial->read() == pgm_read_byte_near(progmemResponseString + matched)) {
                matched++;
                if (matched == responseLength) {
                    return M590_SUCCESS;
                }
            } else
                matched = 0;
        }
    }
    //timeout reached
    return M590_TIMEOUT;
}

//two counters/indexes
m590ResponseCode
M590::readForResponses(const char *progmemResponseString, const char *progmemFailString, const unsigned int timeout) {
    byte passMatched = 0, failMatched = 0;
    byte passResponseLength = strlen_P(progmemResponseString);
    byte failResponseLength = strlen_P(progmemFailString);
    unsigned long startTime = millis();

    while (millis() < (startTime + timeout)) {
        if (_gsmSerial->available()) {
            char c = (char) _gsmSerial->read();

            //check for pass
            if (c == pgm_read_byte_near(progmemResponseString + passMatched)) {
                passMatched++;
                if (passMatched == passResponseLength) {
                    return M590_SUCCESS;
                }
            } else
                passMatched = 0;

            //check for fail
            if (c == pgm_read_byte_near(progmemResponseString + failMatched)) {
                failMatched++;
                if (failMatched == failResponseLength) {
                    return M590_FAILURE;
                }
            } else
                failMatched = 0;
        }
    }
    //timeout reached
    return M590_TIMEOUT;
}

m590ResponseCode
M590::serialToBuffer(char *buffer, const char readUntil, const unsigned int maxBytes, const unsigned int timeout) {
    unsigned long startTime = millis();
    unsigned int bytesRead = 0;

    while (millis() < (startTime + timeout)) {
        if (_gsmSerial->available()) {
            buffer[bytesRead] = (char) _gsmSerial->read();

            //check for readUntil character and replace with 0 (null termination)
            if (buffer[bytesRead] == readUntil) {
                buffer[bytesRead] = 0;
                return M590_SUCCESS;
            }

            bytesRead++;

            if (bytesRead >= (maxBytes - 1)) {
                buffer[bytesRead] = 0;
                return M590_LENGTH_EXCEEDED;
            }
        }
    }
    return M590_TIMEOUT;
}

m590ResponseCode M590::readUntil(const char readUntil, const unsigned int timeout) {
    unsigned long startTime = millis();

    while (millis() < (startTime + timeout)) {
        if (_gsmSerial->available()) {
            if (readUntil == _gsmSerial->read()) {
                return M590_SUCCESS;
            }
        }
    }
    return M590_TIMEOUT;
}



