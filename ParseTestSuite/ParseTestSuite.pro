
# Configuration Settings
CONFIG += console
CONFIG -= app_bundle
QT += widgets network testlib
TARGET = ParseTestSuite
TEMPLATE = app

# Source Files
SOURCES +=		./src/*.cpp ../Parse/*.cpp
HEADERS +=		./src/*.h ../Parse/*.h
INCLUDEPATH +=	../ moc

# Build directories
MOC_DIR = moc

# Kill Debug and Warning Output
DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT
