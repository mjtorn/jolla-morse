# The name of your app.
# NOTICE: name defined in TARGET has a corresponding QML filename.
#         If name defined in TARGET is changed, following needs to be
#         done to match new name:
#         - corresponding QML filename must be changed
#         - desktop icon filename must be changed
#         - desktop filename must be changed
#         - icon definition filename in desktop file must be changed
TARGET = jolla-morse

CONFIG += sailfishapp

SOURCES += src/jolla-morse.cpp

OTHER_FILES += qml/jolla-morse.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    qml/pages/SecondPage.qml \
    rpm/jolla-morse.spec \
    rpm/jolla-morse.yaml \
    jolla-morse.desktop
