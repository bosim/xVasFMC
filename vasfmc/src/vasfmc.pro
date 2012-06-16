# vasfmc can be built with the following options:
#
# QSound as of Qt 4.4.3 only works correctly and as expected on Mac OS.
# Linux requires OpenAL
# Windos requires fmod
#
# Enable support for alternate sound libraries:
# qmake CONFIG+=openal
# qmake CONFIG+=fmod
#
# Default rendering engine is OpenGL on all platforms.
# AGG is an alternative rendering engine also available on all platforms.
# AGG is STRONGLY recommended when building a gauge, due to performance issues.
#
# Enable support for AGG:
# qmake CONFIG+=vas_gl_emul
#
# Enable support for flightgear:
# qmake CONFIG+=withfgfs
#
# Enable support for CPFlight hardware:
# qmake CONFIG+=serial
#

QT += network xml

CONFIG += warn_on release windows thread
CONFIG -= rtti exceptions stl

# The name of the application to be build on all platforms
TARGET = vasfmc
DESTDIR = ..
RC_FILE = vasfmc.rc

# Where to find interesting files...
INCLUDEPATH += ../include
INCLUDEPATH += ../../vaslib/src
DEPENDPATH += ../../vaslib/src
LIBS += -L../../vaslib/lib


# Platform specific settings

# General win32 settings, use msfs and fsuipc includes
win32 {
    DEFINES += HAVE_FSUIPC

    # Special settings for gauge
    gauge {
        # The gauge is built as library instead of application
        TEMPLATE = lib
        CONFIG += shared
        TARGET = vasfmcgau

        PRE_TARGETDEPS += ../../vaslib/lib/libvaslibgau.a
        DEFINES += VASFMC_GAUGE=1 WINVER=0x500
        LIBS += -lvaslibgau -lModuleUser -lpsapi

        QMAKE_RC = windres -D VASFMC_GAUGE=1
    }

    else {
        QMAKE_LINK += /NODEFAULTLIB:libc
        PRE_TARGETDEPS += ../../vaslib/lib/vaslib.lib
        DEFINES += VASFMC_GAUGE=0
        LIBS += vaslib.lib FSUIPC_User.lib wsock32.lib
        #LIBS += -lvaslib -lFSUIPC_User -lws2_32
        #QMAKE_RC = windres -D VASFMC_GAUGE=0
    }
}

else {
	# Settings for non windos platforms
    PRE_TARGETDEPS += ../../vaslib/lib/libvaslib.a
    DEFINES += VASFMC_GAUGE=0
    LIBS += -lvaslib

    ARCH = $$(CPU)
    contains(ARCH,x86_64) {
        DEFINES-= X86_64
    }
    else{
        DEFINES+= X86_64
    }
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


################## FONT STUFF ##################
DEFINES += USE_FONTRENDERER


################## SOUND STUFF ##################
DEFINES += USE_QSOUND

fmod {
    DEFINES += USE_FMOD

    # Disable QQSOUND when using different sound lib
    DEFINES -= USE_QSOUND

    win32 {
        INCLUDEPATH += "../../../src/FMOD SoundSystem/FMOD Programmers API Win32/api/inc"
        LIBPATH += "../../../src/FMOD SoundSystem/FMOD Programmers API Win32/api/lib"
    }

    unix:!macx {
        INCLUDEPATH += "../../../src/fmodapi42002linux/api/inc"
        LIBPATH += "../../../src/fmodapi42002linux/api/lib"
    }

    macx {
        INCLUDEPATH += "/Developer/FMOD\ Programmers\ API/api/inc"
        LIBPATH += "/Developer/FMOD\ Programmers\ API/api/lib"
    }

    # The fmod library itself has the same name on all platforms
    LIBS += -lfmodex
}

openal {
    DEFINES += USE_OPENAL

    # Disable QQSOUND when using different sound lib
    DEFINES -= USE_QSOUND

    LIBS += -lalut -lopenal
}


################## GL EMULATION STUFF ###########
vas_gl_emul {
    DEFINES += VAS_GL_EMUL=1
    INCLUDEPATH += ../../agg-2.5/include
    LIBPATH += ../../agg-2.5/src

    win32 {
        LIBS += -lgdi32 -lagg
    }

    else {
        LIBS += -lagg
    }
}

else {
    DEFINES += VAS_GL_EMUL=0
    QT += opengl
}


################## Flightgear ###########
withfgfs {
    DEFINES += HAVE_PLIB
    LIBS += -lplibnet -lplibul
}


################## qextserialport ###########
serial {
    DEFINES += SERIAL
    INCLUDEPATH += ../../../src/qextserialport-1.2win-alpha
    LIBPATH += ../../../src/qextserialport-1.2win-alpha/build
    LIBS += -lqextserialport

    unix:DEFINES   += _TTY_POSIX_

    win32:DEFINES  += _TTY_WIN_ QWT_DLL QT_DLL
}


################## FILES ##################

precompile_header:!isEmpty(PRECOMPILED_HEADER) {
     DEFINES += USING_PCH
}

HEADERS	+= \
    defines.h \
    fmc_data.h \
    fmc_control_defines.h \
    fmc_control.h \
    fmc_autopilot.h \
    fmc_autopilot_defines.h \
    fmc_autothrottle.h \
    fmc_autothrottle_defines.h \
    fmc_processor.h \
    fmc_navdisplay_defines.h \
    fmc_navdisplay.h \
    fmc_navdisplay_glwidget.h \
    fmc_navdisplay_style.h \
    fmc_navdisplay_style_a.h \
    fmc_navdisplay_style_b.h \
    gldraw.h \
    opengltext.h \
    lfontrenderer.h \
    fmc_cdu.h \
    fmc_cdu_defines.h \
    fmc_cdu_style_a.h \
    fmc_cdu_page_manager.h \
    fmc_cdu_page_base.h \
    fmc_cdu_page.h \
    fmc_cdu_page_menu.h \
    fmc_cdu_page_perf.h \
    fmc_cdu_page_atc.h \
    fmc_console_defines.h \
    fmc_console.h \
    fmc_pfd_defines.h \
    fmc_pfd.h \
    fmc_pfd_glwidget_base.h \
    fmc_pfd_glwidget_style_a.h \
    fmc_pfd_glwidget_style_b.h \
    fmc_ecam_defines.h \
    fmc_ecam.h \
    fmc_ecam_glwidget_base.h \
    fmc_ecam_glwidget_style_a.h \
    fmc_ecam_glwidget_style_b.h \
    fmc_sounds_defines.h \
    fmc_sounds.h \
    fmc_sounds_style_a.h \
    fmc_sounds_style_b.h \
    fmc_sounds_handler.h \
    vas_widget.h \
    vas_gl_widget.h \
    fmc_fcu.h \
    fmc_fcu_defines.h \
    fmc_fcu_style_a.h \
    mmx.h \
    fmc_flightstatus_checker_base.h \
    fmc_flightstatus_checker_style_a.h \
    cpflight_serial.h \
    iocp.h \
    fmc_gps.h \
    fmc_gps_defines.h \
    fmc_gps_style_g.h \
    checklist.h \
    lfontrenderer.h \
    fs9gauges.h

# Extra headers for the gauge
win32:gauge {
    HEADERS	+= \
        fs9gauges.h
}

SOURCES	+= \
    fmc_data.cpp \
    fmc_control.cpp \
    fmc_autopilot.cpp \
    fmc_autothrottle.cpp \
    fmc_processor.cpp \
    fmc_navdisplay.cpp \
    fmc_navdisplay_glwidget.cpp \
    fmc_navdisplay_style.cpp \
    fmc_navdisplay_style_a.cpp \
    fmc_navdisplay_style_b.cpp \
    gldraw.cpp \
    opengltext.cpp \
    lfontrenderer.cpp \
    fmc_cdu.cpp \
    fmc_cdu_style_a.cpp \
    fmc_cdu_page_manager.cpp \
    fmc_cdu_page_base.cpp \
    fmc_cdu_page.cpp \
    fmc_cdu_page_menu.cpp \
    fmc_cdu_page_perf.cpp \
    fmc_cdu_page_atc.cpp \
    fmc_console.cpp \
    fmc_pfd.cpp \
    fmc_pfd_glwidget_base.cpp \
    fmc_pfd_glwidget_style_a.cpp \
    fmc_pfd_glwidget_style_b.cpp \
    fmc_ecam.cpp \
    fmc_ecam_glwidget_base.cpp \
    fmc_ecam_glwidget_style_a.cpp \
    fmc_ecam_glwidget_style_b.cpp \
    fmc_sounds.cpp \
    fmc_sounds_style_a.cpp \
    fmc_sounds_style_b.cpp \
    fmc_sounds_handler.cpp \
    vas_widget.cpp \
    vas_gl_widget.cpp \
    fmc_fcu.cpp \
    fmc_fcu_style_a.cpp \
    mmx.cpp \
    fmc_flightstatus_checker_base.cpp \
    fmc_flightstatus_checker_style_a.cpp \
    cpflight_serial.cpp \
    iocp.cpp \
    fmc_gps.cpp \
    fmc_gps_style_g.cpp \
    main.cpp \
    checklist.cpp \
    lfontrenderer.cpp

# Source files specific for the gauge
win32:gauge {
    SOURCES +=

    SOURCES -= main.cpp
}

# Sources required by AGG
vas_gl_emul {
    SOURCES += \
        vas_gl.cpp \
        vas_gl_backend_qt.cpp \
        vas_gl_backend_agg.cpp

    HEADERS += \
        vas_gl_backend_qt.h \
        vas_gl_backend_agg.h
}

else {
    SOURCES += vas_gl_native.cpp
}

FORMS += \
    ui/fmc_navdisplay.ui \
    ui/fmc_pfd.ui \
    ui/fmc_ecam.ui \
    ui/fmc_cdu.ui \
    ui/fmc_console.ui \
    ui/fmc_fcu.ui \
    ui/fmc_gps.ui
