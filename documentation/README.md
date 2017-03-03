#Documentation
This folder includes some documentation about the Neoway M590 module, bought from [Banggood](http://www.banggood.com/GSM-GPRS-SIM900-1800MHz-Short-Message-Service-m590-SMS-Module-DIY-Kit-For-Arduino-p-1043437.html?p=U530099241512014110R).

##Hardware Specification
Specification | Description
--- | ---
Operating Current | 210mA
Peak Current | 2A
Sleep Mode Current | 2.5mA
Supply Voltage | 3.3V - 4.5V <br>(recommended: 3.9V)
for more specs, look into the [datasheet](m590 datasheet.pdf), page 6.

##Pinout
Because there is no official pinout diagram of the header on the Banggood adapter pcb, I created my own. Here is the pinout of the module:

PCB Pin | M590 Pin | Signal | Description
-------- | ------- | ------ | -----------
1/2 | 1 | GND | Ground
3/4 | 2/3 | VBAT | Vcc (3.3-4.5V)
7 | 7 | RXD | Serial RX (3.3V max)
8 | 8 | TXD | Serial TX (3.3V max)
12 | 10 | RING | Active Low (for signal details, see [datasheet](m590 datasheet.pdf), page 19)
14 | 19 | ON/OFF | Active Low, enables the module <br>(ready after 300ms)

Attention: The I/O level on the pins is only 3.3V **MAX**, because the module operates on an internal reference voltage of 2.85V.  
Supply voltage is 3.3V~4.5V.
![Neoway M590 banggood module pinout](M590%20pinout.jpg?raw=true)


