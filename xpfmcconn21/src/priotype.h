#ifndef priotype_h
#define priotype_h

class PrioType {
public:
    static const unsigned char High = 0;
    static const unsigned char Middle = 0x40;
    static const unsigned char Low = 0x80;
    static const unsigned char Constant= 0xc0;
    static const unsigned short int prioCount = 4;
};

#endif
