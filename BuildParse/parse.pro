
# Configuration Settings
TEMPLATE = lib
CONFIG += staticlib
QT += widgets network
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -stdlib=libc++
HEADERS = ../Parse/*.h
SOURCES = ../Parse/*.cpp

# Mac OS X Configuration
macx {
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}
