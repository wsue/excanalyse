TEMPLATE = lib
TARGET = ExcAnalyse

QTC_SOURCE = D:/tmp/qtc/qt-creator-2.0.1-src/src/
QTC_BUILD  = D:/tmp/qtc/qt-creator-2.0.1-src/build/
#QTC_BUILD  = D:/tools/dev/Qt/qtcreator-1.3.83/

IDE_SOURCE_TREE = $$QTC_SOURCE 
IDE_BUILD_TREE  = $$QTC_BUILD 

LIBS           += -L$$QTC_BUILD/lib/qtcreator/plugins/Nokia \
                  -L$$QTC_BUILD/bin \
                  -L../../out/binutils  \
                  -lparser -lopcodes -lbfd -liberty


include($$QTC_SOURCE/qtcreatorplugin.pri)
include(excanalyse_dependencies.pri)

INCLUDEPATH     += ../include

#   DEFINES += DEMO_PO_LIBRARY
HEADERS += ExcAnalyseplugin.h   \
            navwidget.h         \
            excloaddlg.h        \
            dasmdlg.h
SOURCES += ExcAnalyseplugin.cpp \
            navwidget.cpp       \
            excloaddlg.cpp      \
            dasmdlg.cpp
            
FORMS   += excloaddlg.ui \
            dasmdlg.ui

#	FORMS += findwidget.ui \
#    finddialog.ui
#	RESOURCES += find.qrc

OTHER_FILES += ExcAnalyse.pluginspec
