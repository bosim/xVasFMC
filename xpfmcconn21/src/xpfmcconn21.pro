TEMPLATE = lib

# Shared library without any Qt functionality
QT -= gui core

CONFIG += warn_on release shared
CONFIG -= rtti exceptions thread qt

VERSION = 2.1.0

INCLUDEPATH += ./include/simpleini
INCLUDEPATH += ./include/udp
INCLUDEPATH += ./include/XPLM
INCLUDEPATH += ../../vaslib/src


win32 { 
    TARGET = xpfmcconn/win.xpl
    DEFINES += APL=0 IBM=1 LIN=0 WIN_32
    LIBS += -Llib -lXPLM -lwsock32
}

unix:!macx { 
    TARGET = xpfmcconn/lin.xpl
    DEFINES += APL=0 IBM=0 LIN=1
    QMAKE_CFLAGS += -fstack-protector
    QMAKE_CXXFLAGS += -fstack-protector
}

macx { 
    TARGET = xpfmcconn/mac.xpl
    DEFINES += APL=1 IBM=0 LIN=0
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
    
    # Build for multiple architectures.
    # The following line is only needed to build universal on PPC architectures.
    # QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
    # This line defines for which architectures we build.
    CONFIG += x86 ppc
}

HEADERS += \
    canasoverudp.h \
    datacontainer.h \
    dataref.h \
    datatosend.h \
    logichandler.h \
    owneddata.h \
    simdata.h \
    priotype.h \
    rwtype.h \
    radionav.h \
    apxplane9standard.h \
    fbwjoystickinterface.h \
    xpapiface_msg.h \
    myassert.h \
    navcalc.h \
    communicatorbase.h \
    udpwritesocket.h \
    udpreadsocket.h \
    protocolstreamer.h \
    casprotocol.h \
    paketwriter.h \
    plugin_defines.h \
    ../vaslib/src/fsaccess_xplane_refids.h \
    ../vaslib/src/canas.h

SOURCES += \
    main.cpp \
    canasoverudp.cpp \
    owneddata.cpp \
    simdata.cpp \
    radionav.cpp \
    apxplane9standard.cpp \
    fbwjoystickinterface.cpp \
    logichandler.cpp \
    myassert.cpp \
    navcalc.cpp \
    udpwritesocket.cpp \
    udpreadsocket.cpp \
    casprotocol.cpp
