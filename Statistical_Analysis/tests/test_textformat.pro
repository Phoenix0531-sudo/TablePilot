# QtTest unit-test executable for the pure helpers extracted from MainWindow.
#
# Builds a standalone test binary that links only against textformat.cpp
# (NOT mainwindow.cpp / qcustomplot.cpp / the UI), so it compiles and runs
# fast in CI without needing the full desktop shell or the analysis service.

QT       += core testlib
QT       -= gui widgets

CONFIG   += c++11 console testcase
CONFIG   -= app_bundle

TARGET   = test_textformat
TEMPLATE = app

# Build against the parent directory so #include "textformat.h" resolves.
INCLUDEPATH += $$PWD/..

SOURCES += \
    ../textformat.cpp \
    test_textformat.cpp

HEADERS += \
    ../textformat.h
