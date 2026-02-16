# Qt modules
QT += core network widgets  

# Target name
TARGET = file_sender
TEMPLATE = app

# C++ standard
CONFIG += c++11

# Source files
SOURCES += \
    example_usage.cpp \
    FileSender.cpp

# Header files
HEADERS += \
    FileSender.h

FORMS += \

# Resource files
RESOURCES += \

# Output directories (опционально)
DESTDIR = bin
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

# Platform specific
win32 {
    LIBS += -lws2_32
}
unix {
    LIBS += -lpthread
}

# Debug/Release modes
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
    message("Building debug version")
} else {
    message("Building release version")
}
