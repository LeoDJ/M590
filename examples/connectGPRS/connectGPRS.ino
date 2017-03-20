/*
created March 2017
by Leandro Späth

 This sketch implements the basic functionality of the M590 library.
 It initializes the connection with the module and establishes a GPRS Link
*/

#include <M590.h>

//Standard Settings
#define PIN     "1234"  //enter your PIN for your SIM card here, leave empty for no pin
#define BAUD    9600    //the baud rate for communicating with the M590 module
#define RX_PIN  3       //set the RX Pin of the SoftwareSerial communication (connect to TX on the module)
#define TX_PIN  2       //set the TX Pin (connect to RX on the module)

//APN Settings
#define GPRS_APN    ""  //enter your APN here (e.g. "internet.t-mobile")
#define GPRS_LOGIN  ""  //optional: enter username and password here
#define GPRS_PASS   ""  //     leave blank for no authentication


M590 gsm;
M590GPRSClient client;

void setup()
{
    Serial.begin(115200);
    Serial.println(F("\n\n>> M590 connectGPRS example <<"));
    gsm.begin(BAUD, RX_PIN, TX_PIN); //connect to M590 with 9600 baud, RX pin 3, TX pin 2
    gsm.enableDebugSerial(&Serial); //optionally output debug information on Serial
    gsm.initialize(PIN); //enter your PIN here, leave empty for no pin
}

void loop()
{
    gsm.loop(); //the loop method needs to be called often
    if(gsm.cellularReady()) {
        //initialize GPRS, needs to be implemented
    }
}