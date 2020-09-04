
FLP = $$PWD
INCLUDEPATH += $${FLP}

HEADERS += \
        $${FLP}/fliocnfonthelp.h \
        $${FLP}/flwidget.h

SOURCES += \
        $${FLP}/fliocnfonthelp.cpp \
        $${FLP}/flwidget.cpp


RESOURCES += $${FLP}/res/ \
    $${FLP}/res/fl.qrc
