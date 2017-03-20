/*
 created March 2017
 by Leandro Späth

 This sketch implements the basic functionality of the M590 library.
 It initializes the connection with the module and establishes a cellular link
*/

#include <M590.h>

M590 gsm;

void setup()
{
    Serial.begin(115200);
    Serial.println(F("\n\n>> M590 connectCellular example <<"));
    gsm.begin(9600, 3, 2); //connect to M590 with 9600 baud, RX pin 3, TX pin 2
    gsm.enableDebugSerial(&Serial); //optionally output debug information on Serial
    gsm.initialize("1234"); //enter your PIN here, leave empty for no pin
}

void loop()
{
    gsm.loop(); //the loop method needs to be called often
}