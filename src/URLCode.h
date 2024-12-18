#ifndef _URLCODE_H_
#define _URLCODE_H_

#include <Arduino.h>
#include <String.h>


class URLCode{
private:
    char dec2hex(short int c);
    int hex2dec(char c);
public:
    String urlcode;
    String strcode;
    void urlencode();
    void urldecode();
    void wdtFeed();

};

#endif







