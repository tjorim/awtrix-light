#include "Globals.h"
#include "Preferences.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

Preferences Settings;

char *getID()
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char *macStr = new char[24];
    snprintf(macStr, 24, "awtrix_%02x%02x%02x", mac[3], mac[4], mac[5]);
    return macStr;
}

void startLittleFS()
{
    if (LittleFS.begin())
    {
        LittleFS.mkdir("/MELODIES");
        LittleFS.mkdir("/ICONS");
    }
    else
    {
        Serial.println("ERROR on mounting LittleFS. It will be formmatted!");
        LittleFS.format();
        ESP.restart();
    }
}

void loadDevSettings()
{
    Serial.println("laodSettings");
    File file = LittleFS.open("/dev.json", "r");
    if (!file)
    {
        return;
    }
    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        Serial.println(F("Failed to read dev settings"));
        return;
    }

    if (doc.containsKey("bootsound"))
    {
        BOOT_SOUND = doc["bootsound"].as<String>();
    }

    if (doc.containsKey("bootsound"))
    {
        UPPERCASE_LETTERS = doc["uppercase"].as<bool>();
    }

    file.close();
}

void loadSettings()
{
    startLittleFS();
    Settings.begin("awtrix", false);
    MATRIX_FPS = Settings.getUChar("FPS", 23);
    BRIGHTNESS = Settings.getUChar("BRI", 120);
    AUTO_BRIGHTNESS = Settings.getBool("ABRI", true);
    TEXTCOLOR_565 = Settings.getUInt("COL", 0xFFFF);
    AUTO_TRANSITION = Settings.getBool("TRANS", true);
    TIME_PER_TRANSITION = Settings.getUInt("TSPEED", 500);
    TIME_PER_APP = Settings.getUInt("ADUR", 5000);
    TIME_FORMAT = Settings.getString("TFORMAT", "%H:%M:%S");
    DATE_FORMAT = Settings.getString("DFORMAT", "%d.%m.%y");
    START_ON_MONDAY = Settings.getBool("SOM", true);
    IS_CELSIUS = Settings.getBool("CEL", true);
    SHOW_TIME = Settings.getBool("TIM", true);
    SHOW_DATE = Settings.getBool("DAT", true);
    SHOW_TEMP = Settings.getBool("TEMP", true);
    SHOW_HUM = Settings.getBool("HUM", true);
    MATRIX_LAYOUT = Settings.getUInt("MAT", 0);
#ifdef ULANZI
    SHOW_BAT = Settings.getBool("BAT", true);
#endif
    SOUND_ACTIVE = Settings.getBool("SOUND", true);
    Settings.end();
    uniqueID = getID();
    MQTT_PREFIX = String(uniqueID);
    loadDevSettings();
}

void saveSettings()
{
    Settings.begin("awtrix", false);
    Settings.putUChar("FPS", MATRIX_FPS);
    Settings.putUChar("BRI", BRIGHTNESS);
    Settings.putBool("ABRI", AUTO_BRIGHTNESS);
    Settings.putBool("TRANS", AUTO_TRANSITION);
    Settings.putUInt("COL", TEXTCOLOR_565);
    Settings.putUInt("TSPEED", TIME_PER_TRANSITION);
    Settings.putUInt("ADUR", TIME_PER_APP);
    Settings.putString("TFORMAT", TIME_FORMAT);
    Settings.putString("DFORMAT", DATE_FORMAT);
    Settings.putBool("SOM", START_ON_MONDAY);
    Settings.putBool("CEL", IS_CELSIUS);
    Settings.putBool("TIM", SHOW_TIME);
    Settings.putBool("DAT", SHOW_DATE);
    Settings.putBool("TEMP", SHOW_TEMP);
    Settings.putBool("HUM", SHOW_HUM);
    Settings.putUInt("MAT", MATRIX_LAYOUT);
#ifdef ULANZI
    Settings.putBool("BAT", SHOW_BAT);
#endif
    Settings.putBool("SOUND", SOUND_ACTIVE);
    Settings.end();
}

const char *uniqueID;
IPAddress local_IP;
IPAddress gateway;
IPAddress subnet;
IPAddress primaryDNS;
IPAddress secondaryDNS;
const char *VERSION = "0.47";
String MQTT_HOST = "";
uint16_t MQTT_PORT = 1883;
String MQTT_USER;
String MQTT_PASS;
String MQTT_PREFIX;
String CITY = "Berlin,de";
bool IO_BROKER = false;
bool NET_STATIC = false;
bool SHOW_TIME = true;
bool SHOW_DATE = true;
bool SHOW_WEATHER = true;
#ifdef ULANZI
bool SHOW_BAT = true;
#endif
bool SHOW_TEMP = true;
bool SHOW_HUM = true;
bool SHOW_SECONDS = true;
bool SHOW_WEEKDAY = true;
String NET_IP = "192.168.178.10";
String NET_GW = "192.168.178.1";
String NET_SN = "255.255.255.0";
String NET_PDNS = "8.8.8.8";
String NET_SDNS = "1.1.1.1";
int TIME_PER_APP = 7000;
uint8_t MATRIX_FPS = 23;
int TIME_PER_TRANSITION = 500;
String NTP_SERVER = "de.pool.ntp.org";
String NTP_TZ = "CET-1CEST,M3.5.0,M10.5.0/3";
bool HA_DISCOVERY = false;

// Periphery
String CURRENT_APP;
float CURRENT_TEMP;
bool IS_CELSIUS;
float CURRENT_HUM;
float CURRENT_LUX;
uint8_t BRIGHTNESS = 120;
uint8_t BRIGHTNESS_PERCENT;
#ifdef ULANZI
uint8_t BATTERY_PERCENT;
uint16_t BATTERY_RAW;
#endif
uint16_t LDR_RAW;
String TIME_FORMAT = "%H:%M:%S";
String DATE_FORMAT = "%d.%m.%y";
bool START_ON_MONDAY;

String ALARM_SOUND;
uint8_t SNOOZE_TIME;

String TIMER_SOUND;

// Matrix States
bool AUTO_TRANSITION = false;
bool AUTO_BRIGHTNESS = true;
bool UPPERCASE_LETTERS = true;
bool AP_MODE;
bool MATRIX_OFF;
bool TIMER_ACTIVE;
bool ALARM_ACTIVE;
uint16_t TEXTCOLOR_565 = 0xFFFF;
bool SOUND_ACTIVE;
String BOOT_SOUND = "";
uint8_t VOLUME;
uint8_t VOLUME_PERCENT;
int MATRIX_LAYOUT;
bool UPDATE_AVAILABLE = false;