#ifndef rwtype_h
#define rwtype_h

class RWType {
public:
    static const short ReadWrite = 1 | 2;
    static const short WriteOnly = 2;
    static const short ReadOnly = 1;
};

#endif
