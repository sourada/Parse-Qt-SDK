
# Configuration Settings
CONFIG += console
QT += widgets network testlib
TARGET = ParseTestSuite
TEMPLATE = app

# Source Files
SOURCES +=		./src/*.cpp ../Parse/*.cpp
HEADERS +=		./src/*.h ../Parse/*.h
INCLUDEPATH +=	../Parse moc

# Build directories
MOC_DIR = moc

# Kill Debug and Warning Output
DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

# Mac Settings
macx {
	CONFIG -= app_bundle
}
