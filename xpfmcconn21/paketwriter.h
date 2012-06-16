#ifndef PAKETWRITER_H
#define PAKETWRITER_H

class PaketWriter
{
public:
    PaketWriter(){}
    virtual ~PaketWriter() {}
    virtual long write(const void* data, size_t size) = 0;
};

#endif // PAKETWRITER_H
