/*
created March 2017
by Leandro Späth
*/

#include <M590.h>

M590 gsm;

void setup()
{
    Serial.begin(115200);
    Serial.println(F("\n\n>> M590 connectCellular example <<"));
    gsm.begin(9600, 3, 2); //connect to M590 with 19200 baud, RX pin 3, TX pin 2
    gsm.enableDebugSerial(&Serial); //optionally output debug information on Serial
    //gsm.initialize("1234"); //enter your PIN here, leave empty for no pin
}

void loop()
{
    gsm.loop(); //the loop method needs to be called often
    if(Serial.available())
        gsm.write(Serial.read());
    if(gsm.available())
        Serial.write(gsm.read());
}