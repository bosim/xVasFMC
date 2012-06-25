# vaslib can be built with the following options:
#
# Enable support for flightgear:
# qmake CONFIG+=withfgfs
#
# Enable support for vascharts:
# qmake CONFIG+=withvascharts
#
# Enable support for profiling with gprof and valgrind:
# qmake CONFIG+=profile

TEMPLATE = lib

QT += network xml

CONFIG += warn_on release static
CONFIG -= rtti exceptions stl thread

# The name of the library to be build on all platforms
TARGET = vaslib
DESTDIR = ../lib
RC_FILE = vaslib.rc


# Platform specific settings

# General win32 settings, use msfs and fsuipc includes
win32 {

HEADERS += \
    code_timer.h

SOURCES += \
    code_timer.cpp

}


# Special settings for Mac OS
macx {
    # Build for multiple architectures.
    # The following line is only needed to build universal on PPC architectures.
    # QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk

    # This line defines for which architectures we build.
    CONFIG += x86 ppc
}


# Optional build settings

# Settings that enable profiling with gprof
profile {
    QMAKE_CXXFLAGS += -pg -g -O3 -DNDEBUG
    QMAKE_LFLAGS += -pg -g
}


# Files to be included for reading XML
commonxml {
    HEADERS += xml.model.h 
    SOURCES += xml.model.cpp
}


# Files to be included for flightgear aka fgfs aka fg access
withfgfs {
    CONFIG += commonxml
    DEFINES += HAVE_PLIB
    HEADERS +=
           
    SOURCES +=
}


# The files constituting vaslib itself
HEADERS += \
    ptrlist.h \
    assert.h \
    config.h \
    configwidget.h \
    logger.h \
    meanvalue.h \
    median.h \
    pushbutton.h \
    mouse_input_area.h \
    smoothing.h \
    waypoint.h \
    waypoint_hdg_to_alt.h \
    waypoint_hdg_to_intercept.h \
    airport.h \
    runway.h \
    intersection.h \
    ndb.h \
    vor.h \
    ils.h \
    holding.h \
    route.h \
    airway.h \
    procedure.h \
    procedure_serialization.h \
    sid.h \
    star.h \
    transition.h \
    approach.h \
    flightroute.h \
    flightstatus.h \
    fsaccess.h \
    navcalc.h \
    navdata.h \
    gshhs.h \
    geodata.h \
    weather.h \
    projection.h \
    projection_mercator.h \
    projection_greatcircle.h \
    transport_layer_iface.h \
    transport_layer_tcpclient.h \
    transport_layer_tcpserver.h \ 
    transport_layer_tcpserver_clientbuffer.h \
#    serialization_layer_iface.h \
#    serialization_layer_plain.h \
    containerfactory.h \
    containerbase.h \
    infodlgimpl.h \
    declination.h \
    declination_geomag.h \
    vroute.h \
    vas_path.h \
    #fsaccess_xplane_defines.h \
    #fsaccess_xplane.h \
    trend.h \
    controller_base.h \
    controller_throttle.h \
    controller_speed.h \
    fly_by_wire.h \
    flight_mode_tracker.h \
    statistics.h \
    aircraft_data.h \
    serialization_iface.h \
    fmc_data_provider.h \
    bithandling.h \
    noise_generator.h \
    declination_geomag.h \
    fsaccess_xplane_refids.h

# Disable X-Plane access in gauge
!gauge {
    SOURCES += fsaccess_xplane.cpp
    HEADERS += \
        fsaccess_xplane_defines.h \
        fsaccess_xplane.h \
        canas.h
}

SOURCES += \
    assert.cpp \
    config.cpp \
    configwidget.cpp \
    logger.cpp \
    pushbutton.cpp \
    mouse_input_area.cpp \
    smoothing.cpp \
    waypoint.cpp \
    airport.cpp \
    runway.cpp \
    intersection.cpp \
    ndb.cpp \
    vor.cpp \
    ils.cpp \
    holding.cpp \
    route.cpp \
    airway.cpp \
    procedure.cpp \
    procedure_serialization.cpp \
    sid.cpp \
    star.cpp \
    transition.cpp \
    approach.cpp \
    flightroute.cpp \
    flightstatus.cpp \
    fsaccess.cpp \
    navcalc.cpp \
    navdata.cpp \
    geodata.cpp \
    weather.cpp \
    projection_mercator.cpp \
    projection_greatcircle.cpp \
    transport_layer_iface.cpp \
    transport_layer_tcpclient.cpp \
    transport_layer_tcpserver.cpp \ 
    transport_layer_tcpserver_clientbuffer.cpp \
#    serialization_layer_iface.cpp \
#    serialization_layer_plain.cpp \
    containerfactory.cpp \
    containerbase.cpp \
    infodlgimpl.cpp \
    declination.cpp \
    declination_geomag.cpp \
    vroute.cpp \
    vas_path.cpp \
    #fsaccess_xplane.cpp \
    controller_throttle.cpp \
    controller_speed.cpp \
    fly_by_wire.cpp \
    flight_mode_tracker.cpp \
    statistics.cpp \
    aircraft_data.cpp \
    noise_generator.cpp \
    declination_geomag.cpp

FORMS = \
    waypointchoosedlg.ui \
    routedlg.ui \
    infodlg.ui \
    latlonpointdlg.ui \
    pbdpointdlg.ui \
    textelemdlg.ui
