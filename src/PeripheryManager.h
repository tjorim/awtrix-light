#ifndef PeripheryManager_h
#define PeripheryManager_h

#include <Arduino.h>
#include <EasyButton.h>
#ifdef ULANZI
    #include "Adafruit_SHT31.h"
#else
    #include "Adafruit_BME280.h"
    #include "SoftwareSerial.h"
    #include <DFMiniMp3.h>
#endif

class PeripheryManager_
{
private:
    PeripheryManager_() = default;
    void checkAlarms();
#ifdef ULANZI
    const int BatReadings = 10;
    uint16_t TotalBatReadings[10];
#endif
    int readIndex = 0;
    uint16_t total = 0;
    uint16_t average = 0;
    const int LDRReadings = 30;
    uint16_t TotalLDRReadings[30];
    int sampleIndex = 0;
    unsigned long previousMillis = 0;
    const unsigned long interval = 1000;

public:
    static PeripheryManager_ &getInstance();
    void setup();
    void tick();
    void playBootSound();
    void playFromFile(String file);
    bool isPlaying();
    void stopSound();
    void setVolume(uint8_t);
    const char *readUptime();
};

extern PeripheryManager_ &PeripheryManager;
#endif
