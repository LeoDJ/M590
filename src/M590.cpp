/*
created March 2017
by Leandro Späth
*/

#include <Arduino.h>
#include <M590.h>


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

bool M590::cellularReady() {
    return (_currentState == M590_STATE_CELLULAR_CONNECTED);
}

void M590::shutdown() {
    sendCommand(M590_COMMAND_SHUTDOWN);
    _currentState = M590_STATE_SHUTDOWN;
}

m590States M590::getCurrentState() {
    return _currentState;
}

bool M590::initialize(String pin) {
    if (!checkAlive()) {//checkAlive still gets executed
        printDebug(M590_ERROR_NOT_RESPONDING); //TODO: better error handling
        return false;
    }

    checkPinRequired();
    if (_currentState == M590_STATE_PIN_REQUIRED) {
        if (pin && pin != "")
            sendPinEntry(pin); //sets state to pin_entry_done, when successful
        else {
            printDebug(M590_ERROR_NO_PIN);
            return false;
        }
    } else if (_currentState == M590_STATE_FATAL_ERROR)
        return false;

    if (_currentState == M590_STATE_PIN_ENTRY_DONE) {
        _currentState = M590_STATE_PIN_VALIDATION;
        readForAsyncResponse(M590_RESPONSE_PIN_VAL_DONE); //start asnyc reading (execution continued in loop())
    } else if (_currentState == M590_STATE_PIN_VALIDATION_DONE) {
        printDebug(M590_LOG_NO_PIN_REQUIRED);
    } else {
        _currentState = M590_STATE_FATAL_ERROR;
        printDebug(M590_ERROR_WRONG_PIN);
    }
}

//thee loop gets called every arduino code to handle "async" responses
void M590::loop() {
    switch (_currentState) {
        case M590_STATE_STARTUP_DONE:
            break;

        case M590_STATE_PIN_VALIDATION: {
            m590ResponseCode status = readForAsyncResponse(); //call function with last entered parameters
            if (status == M590_SUCCESS)
                _currentState = M590_STATE_PIN_VALIDATION_DONE;
            else if (status == M590_TIMEOUT) {
                _currentState = M590_STATE_FATAL_ERROR;
                printDebug(M590_ERROR_PINVAL_TIMEOUT);
            }
            break;
        }

        case M590_STATE_PIN_VALIDATION_DONE: {
            _currentState = M590_STATE_CELLULAR_CONNECTING;
            break;
        }

        case M590_STATE_CELLULAR_CONNECTING: {
            unsigned long curMillis = millis();
            if (_asyncStartTime == 0) _asyncStartTime = curMillis; //repurpose asyncStartTime variable
            else if (curMillis >= _asyncStartTime + STATUS_POLLING_RATE) {
                m590NetworkStates netState = checkNetworkState();
                if (netState == M590_NET_REGISTERED)
                    _currentState = M590_STATE_CELLULAR_CONNECTED;
                else if (netState == M590_NET_SEARCHING_NOT_REGISTERED) {
                    printDebug(F(".")); //print dots to show wait for registration
                } else {
                    _currentState = M590_STATE_FATAL_ERROR;
                    printDebug(M590_ERROR_UNHANDLED_NET_STATE);
                    printDebug(String(netState), true);
                }
                _asyncStartTime = curMillis;
            }
            break;
        }

        case M590_STATE_FATAL_ERROR: {
            //reset the library and try again
            break;
        }
    }
    if (_previousState != _currentState) {
        printDebug(M590_LOG[_currentState]);
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

bool M590::checkPinRequired() {
    if (_currentState == M590_STATE_STARTUP_DONE) {
        sendCommand(M590_COMMAND_CHECK_PIN);
        memset(_responseBuffer, 0, sizeof(_responseBuffer));
        readForResponse(M590_RESPONSE_OK, _responseBuffer, sizeof(_responseBuffer));
        bool required = bufferStartsWithProgmem(_responseBuffer, M590_RESPONSE_PIN_REQUIRED);
        //check if module does not need pin entry
        bool alreadyReady = bufferStartsWithProgmem(_responseBuffer, M590_RESPONSE_PIN_DONE);
        _currentState = required ? M590_STATE_PIN_REQUIRED : M590_STATE_PIN_VALIDATION_DONE;
        if (!required && !alreadyReady) {
            _currentState = M590_STATE_FATAL_ERROR;
            printDebug(M590_ERROR_OTHER_PIN_ERR);
        }
        return required; //returns true, if pin is required
    } else return false;
}

bool M590::sendPinEntry(String pin, void (*callback)(void)) {
    if (_currentState == M590_STATE_PIN_REQUIRED) {
        _gsmSerial->print((__FlashStringHelper *) M590_COMMAND_PREFIX);
        _gsmSerial->print((__FlashStringHelper *) M590_COMMAND_INPUT_PIN);
        _gsmSerial->print('"');
        _gsmSerial->print(pin);
        _gsmSerial->print('"');
        _gsmSerial->println();
        bool success = readForResponses(M590_RESPONSE_OK, M590_RESPONSE_FAIL) == M590_SUCCESS;
        if (success) _currentState = M590_STATE_PIN_ENTRY_DONE;
        return success;
    }
    return false;
}


m590NetworkStates M590::checkNetworkState() {
    sendCommand(M590_COMMAND_CHECK_NETWORK_STATUS);
    memset(_responseBuffer, 0, sizeof(_responseBuffer));
    m590ResponseCode r = readForResponse(M590_RESPONSE_OK, _responseBuffer, sizeof(_responseBuffer));
    //the fourth char in the response (e.g. " 0,3") will be the registration state (e.g. 3)
    if (r == M590_SUCCESS)
        return (m590NetworkStates) (_responseBuffer[3] - '0'); //convert to integer, maps to m590NetworkStates
    else return M590_NET_PARSE_ERROR;
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

void M590::resetAsyncVariables() {
    _asyncStartTime = 0;
    _asyncBytesMatched = 0;
    _asyncResponseLength = 0;
    _asyncProgmemResponseString = NULL;
}

m590ResponseCode M590::readForAsyncResponse(const char *progmemResponseString, const unsigned int timeout) {
    if (_asyncStartTime == 0)
        _asyncStartTime = millis();
    if (!_asyncProgmemResponseString && progmemResponseString) {
        //save responseString pointer to look for in private variable (so async function can be called without parameters)
        _asyncProgmemResponseString = progmemResponseString;
        _asyncResponseLength = strlen_P(progmemResponseString);
    } else if (!_asyncProgmemResponseString && !progmemResponseString) {
        //return when function is called for the first time without any parameters
        return M590_NO_PARAMETERS;
    }
    if (millis() > _asyncStartTime + timeout) {
        resetAsyncVariables();
        return M590_TIMEOUT;
    }

    while (_gsmSerial->available()) {
        char c = (char) _gsmSerial->read();
        if (c == pgm_read_byte_near(_asyncProgmemResponseString + _asyncBytesMatched)) {
            _asyncBytesMatched++;
            if (_asyncBytesMatched == _asyncResponseLength) {
                resetAsyncVariables();
                return M590_SUCCESS;
            }
        } else
            _asyncBytesMatched = 0;
    }
    return M590_ASYNC_RUNNING;

}

m590ResponseCode M590::readForResponse(const char *progmemResponseString, char *buffer, const unsigned int max_bytes,
                                       const unsigned int timeout) {
    byte dataMatched = 0, dataRead = 0;
    byte readingData = 0; //state
    byte dataLength = strlen_P(M590_RESPONSE_PREFIX);
    byte matched = 0;
    byte responseLength = strlen_P(progmemResponseString);

    unsigned long startTime = millis();

    while (millis() < (startTime + timeout)) {
        if (_gsmSerial->available()) {
            char c = (char) _gsmSerial->read();
            if (c == pgm_read_byte_near(M590_RESPONSE_PREFIX + dataMatched)) { //check if a return data message begins
                dataMatched++;
                if (dataMatched == dataLength) { //when return data begins
                    readingData = 1;
                    dataMatched = 0;
                }
            } else {
                dataMatched = 0;
            }
            if (readingData) { //if in reading return data mode

                if (readingData == 1 && c == ':') //before colon, there is only the command echoed back
                    readingData = 2;
                else if (readingData == 2) { //if at actual data
                    //_debugSerial->print(c);
                    if (c == '\r') { //if reached end of return data
                        readingData = 0;
                    } else {
                        buffer[dataRead] = c;
                        dataRead++;
                        if (dataRead >= (max_bytes - 1)) { //if read more than buffer size
                            buffer[dataRead] = 0;
                            return M590_LENGTH_EXCEEDED;
                        }
                    }
                }
            } else {
                if (c == pgm_read_byte_near(progmemResponseString + matched)) {
                    matched++;
                    if (matched == responseLength) {
                        return M590_SUCCESS;
                    }
                } else
                    matched = 0;

            }
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

bool M590::bufferStartsWithProgmem(char *buffer, const char *progmemString) {
    bool matches = true;
    for (int i = 0; i < strlen_P(progmemString); i++) {
        matches = buffer[i] == pgm_read_byte_near(progmemString + i);
    }
    return matches;
}

void M590::printDebug(const char *progmemString, bool withNewline) {
    if (_debugSerial) {
        _debugSerial->print((__FlashStringHelper *) progmemString);
        if (withNewline) _debugSerial->println();
    }
}

void M590::printDebug(const String s, bool withNewline) {
    if (_debugSerial) {
        _debugSerial->print(s);
        if (withNewline) _debugSerial->println();
    }
}
