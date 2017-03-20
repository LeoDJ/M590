//
// Created by Leandro on 20.03.2017.
//

#include "M590_GPRSClient.h"

bool M590GPRSClient::initialize(String APN, String user, String pass) {
    //Not sure, if I really want to seperate it
    //only means more work keeping them seperate and implementing an interface between the two
    //on the other hand, having a layer of abstraction seems better
    //client.available() looks better than gsm.gprsAvailable() ...
}
