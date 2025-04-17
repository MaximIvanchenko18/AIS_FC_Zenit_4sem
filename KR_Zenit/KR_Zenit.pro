QT       += core gui
QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    coachstaff.cpp \
    footballclubs.cpp \
    main.cpp \
    mainwindow.cpp \
    matches.cpp \
    matchesinsquads.cpp \
    menu.cpp \
    offeredplayers.cpp \
    players.cpp \
    playersinsquads.cpp \
    transfers.cpp

HEADERS += \
    coachstaff.h \
    footballclubs.h \
    mainwindow.h \
    matches.h \
    matchesinsquads.h \
    menu.h \
    offeredplayers.h \
    players.h \
    playersinsquads.h \
    transfers.h

FORMS += \
    coachstaff.ui \
    footballclubs.ui \
    mainwindow.ui \
    matches.ui \
    matchesinsquads.ui \
    menu.ui \
    offeredplayers.ui \
    players.ui \
    playersinsquads.ui \
    transfers.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
