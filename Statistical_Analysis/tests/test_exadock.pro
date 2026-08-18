# QtTest unit-test executable for the Exa API response parser.
#
# Builds a standalone test binary that links against webresearchdock.cpp (which
# provides the static WebResearchDock::parseResults) plus the Qt modules it
# transitively needs (core/testlib for the test framework, network for
# QNetworkAccessManager/QJsonDocument, widgets for the QDockWidget base class
# declared in the header). gui is pulled in by widgets. The full MainWindow,
# QCustomPlot, and the analysis service are NOT linked here, so the test stays
# fast and isolated.

QT       += core testlib network widgets
QT       += gui

CONFIG   += c++11 console testcase
CONFIG   -= app_bundle

TARGET   = test_exadock
TEMPLATE = app

INCLUDEPATH += $$PWD/..

SOURCES += \
    ../webresearchdock.cpp \
    test_exadock.cpp

HEADERS += \
    ../webresearchdock.h
