QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/64bit -lcomponents
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/64bit -lcomponents

#INCLUDEPATH += $$PWD/components/material
#DEPENDPATH += $$PWD/components/material

SOURCES += \
    filefinderwidget.cpp \
    main.cpp \
    mainwindow.cpp \


HEADERS += \
    filefinderwidget.h \
    filesearchtask.h \
    mainwindow.h \


FORMS += \
    filefinderwidget.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qdarkstyle/dark/style.qrc

DISTFILES += \
    qdarkstyle/dark/style.qss
