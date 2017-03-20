//
// Created by Leandro on 20.03.2017.
//

#ifndef M590_GPRSCLIENT_H
#define M590_GPRSCLIENT_H

#include <M590.h>


class M590GPRSClient {
public:
    bool initialize(String APN, String user = "", String pass = "");
};


#endif //M590_M590GPRSCLIENT_H
