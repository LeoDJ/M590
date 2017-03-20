//
// Created by Leandro on 20.03.2017.
//

#ifndef M590_CONSTANTS_H
#define M590_CONSTANTS_H


//=============================================================
// Data structures
//=============================================================


enum m590States {
    M590_STATE_FATAL_ERROR,                 //0
    M590_STATE_SHUTDOWN,
    M590_STATE_STARTUP,
    M590_STATE_STARTUP_DONE,
    M590_STATE_PIN_REQUIRED,
    M590_STATE_PIN_ENTRY_DONE,              //5
    M590_STATE_PIN_VALIDATION,
    M590_STATE_PIN_VALIDATION_DONE,
    M590_STATE_CELLULAR_CONNECTING,
    M590_STATE_CELLULAR_CONNECTED,
};

enum m590ResponseCode {
    M590_SUCCESS,
    M590_FAILURE,
    M590_TIMEOUT,
    M590_LENGTH_EXCEEDED,
    M590_ASYNC_RUNNING,
    M590_NO_PARAMETERS,
    M590_UNDEFINED
};

enum m590NetworkStates {
    M590_NET_NOT_REGISTERED_NOT_SEARCHING,
    M590_NET_REGISTERED,
    M590_NET_REGISTRATION_REFUSED,
    M590_NET_SEARCHING_NOT_REGISTERED,
    M590_NET_UNKNOWN,
    M590_NET_REGISTERED_ROAMING,
    M590_NET_PARSE_ERROR, //not actually part of response, used to determine function failure
};


//=============================================================
// Commands needed for serial communication
//=============================================================


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
        M590_RESPONSE_PREFIX[]          PROGMEM = "+",//"\r\n+",
        M590_RESPONSE_SEPERATOR[]       PROGMEM = ": ",
        M590_RESPONSE_OK[]              PROGMEM = "OK\r\n",
        M590_RESPONSE_ERROR[]           PROGMEM = "ERROR\r\n",
        M590_RESPONSE_FAIL[]            PROGMEM = "FAIL\r\n",
        M590_RESPONSE_PIN_REQUIRED[]    PROGMEM = " SIM PIN",
        M590_RESPONSE_PIN_DONE[]        PROGMEM = " READY",
        M590_RESPONSE_PIN_VAL_DONE[]    PROGMEM = "+PBREADY";

const char
        M590_AT[]                       PROGMEM = "AT",
        M590_CRLF[]                     PROGMEM = "\r\n",
        M590_COMMAND_PREFIX[]           PROGMEM = "AT+",
        M590_CONTENT_LENGTH_HEADER[]    PROGMEM = "Content-Length: ";


//=============================================================
// Messages for debug logging
//=============================================================

const char
        M590_ERROR_NOT_RESPONDING[]         PROGMEM = "\nThe M590 did not respond to an \"AT\". Please check serial connection, power supply and ONOFF pin.",
        M590_ERROR_NO_PIN[]                 PROGMEM = "\nNo pin was specified, but the module requests one",
        M590_ERROR_WRONG_PIN[]              PROGMEM = "\nWrong PIN was entered, down one try.",
        M590_ERROR_OTHER_PIN_ERR[]          PROGMEM = "\nError during PIN check, maybe a PUK is required, please check SIM card in a phone",
        M590_ERROR_PINVAL_TIMEOUT[]         PROGMEM = "\nTimeout during pin validation, please check module and try again",
        M590_ERROR_UNHANDLED_NET_STATE[]    PROGMEM = "\nNetwork status returned unhandled state: ";


const char
        M590_LOG_NO_PIN_REQUIRED[]      PROGMEM = "No PIN was required";

const char
        M590_LOG_00[]   PROGMEM = "Shutdown",
        M590_LOG_01[]   PROGMEM = "In Startup",
        M590_LOG_02[]   PROGMEM = "Module is active.",
        M590_LOG_03[]   PROGMEM = "Pin entry is required",
        M590_LOG_04[]   PROGMEM = "Pin entry successful",
        M590_LOG_05[]   PROGMEM = "Pin is being validated",
        M590_LOG_06[]   PROGMEM = "Pin validation successful",
        M590_LOG_07[]   PROGMEM = "Registering on cellular network",
        M590_LOG_08[]   PROGMEM = "Connected to cellular network",
        M590_LOG_09[]   PROGMEM = "A fatal error occured, library can not continue";

//crude method of accessing multiple progmem Strings easily // index corresponds to m590States
static const char *M590_LOG[] = {
        M590_LOG_09, //fatal moved to 0
        M590_LOG_00,
        M590_LOG_01,
        M590_LOG_02,
        M590_LOG_03,
        M590_LOG_04,
        M590_LOG_05,
        M590_LOG_06,
        M590_LOG_07,
        M590_LOG_08,
};


#endif //M590_M590CONSTANTS_H
