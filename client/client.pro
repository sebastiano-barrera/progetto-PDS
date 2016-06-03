#-------------------------------------------------
#
# Project created by QtCreator 2016-05-31T10:45:51
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app

INCLUDEPATH += ../proto/cpp ../extern/include

SOURCES += main.cpp\
        mainwindow.cpp \
    connectionwidget.cpp \
    messagestream.cpp \
    clientprotocol.cpp

SOURCES += ../proto/cpp/protocol.pb.cc \
    ../proto/cpp/keys.pb.cc

HEADERS  += mainwindow.h \
    connectionwidget.h \
    messagestream.h \
    clientprotocol.h

FORMS    += mainwindow.ui \
    connectionwidget.ui


INCLUDEPATH += $$PWD/../extern/include $$PWD/../proto/cpp
DEPENDPATH += $$PWD/../extern/include
     CONFIG(release, debug|release):LIBS += $$PWD/../extern/lib/libprotobuf.lib
else:CONFIG(debug,   debug|release):LIBS += $$PWD/../extern/lib/libprotobufd.lib

win32-msvc* {
    MSVC_VER = $$(VisualStudioVersion)
    equals(MSVC_VER, 14.0){
        # After the introduction of the Universal CRT, these include paths and libs are needed,
        # and they aren't automatically set by the compiler.
        message( "Microsoft Visual Studio 14 2015 detected => enabling Universal CRT" )

        # TODO Support x64 as well?
        INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/ucrt"
             CONFIG(release, debug|release): LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10240.0/ucrt/x86/ucrt.lib"
        else:CONFIG(debug,   debug|release): LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10240.0/ucrt/x86/ucrtd.lib"
    }
}
