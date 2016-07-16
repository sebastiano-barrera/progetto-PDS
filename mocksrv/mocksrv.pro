QT += core network
QT -= gui

CONFIG += c++11

TARGET = mocksrv
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += messagestream.h \
    mockserverprotocol.h \
    windowinfo.h
SOURCES += main.cpp \
    messagestream.cpp \
    mockserverprotocol.cpp

SOURCES += ../proto/cpp/protocol.pb.cc \
    ../proto/cpp/keys.pb.cc


INCLUDEPATH += $$PWD/../extern/include $$PWD/../proto/cpp
DEPENDPATH += $$PWD/../extern/include

unix* {
    LIBS += $$PWD/../extern/lib/libprotobuf.a
}

win32-msvc* {
    LIBS += $$PWD/../extern/lib/libprotobufd.lib

    MSVC_VER = $$(VisualStudioVersion)
    equals(MSVC_VER, 14.0){
        # After the introduction of the Universal CRT, these include paths and libs are needed,
        # and they aren't automatically set by the compiler.
        message( "Microsoft Visual Studio 14 2015 detected => enabling Universal CRT" )

        # TODO Support x64 as well
        INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/ucrt"
        #LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10240.0/ucrt/x86/ucrt.lib"
        LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10240.0/ucrt/x86/ucrtd.lib"
    }
}
