#ifndef AppS_H
#define AppS_H

#include <vector>
#include <map>
#include "icons.h"
#include <FastLED_NeoMatrix.h>
#include "MatrixDisplayUi.h"
#include "Globals.h"
#include "Functions.h"
#include "MenuManager.h"
#include "PeripheryManager.h"
#include "DisplayManager.h"
#include "LittleFS.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Ticker.h>

Ticker downloader;

tm timeInfo;
uint16_t nativeAppsCount;
uint16_t customPagesCount;

int WEATHER_CODE;
String WEATHER_TEMP;
String WEATHER_HUM;

struct CustomApp
{
    int16_t scrollposition = 0;
    int16_t scrollDelay = 0;
    String text;
    uint16_t color;
    File icon;
    bool isGif;
    bool rainbow;
    bool soundPlayed;
    uint16_t duration = 0;
    String sound;
    int16_t repeat = 0;
    int16_t currentRepeat = 0;
    String name;
    byte pushIcon = 0;
    int16_t iconPosition = 0;
    bool iconWasPushed = false;
    int barData[16] = {0};
    int barSize;
};

String currentCustomApp;
std::map<String, CustomApp> customApps;

struct Notification
{
    int16_t scrollposition = 34;
    int16_t scrollDelay = 0;
    String text;
    uint16_t color;
    File icon;
    bool rainbow;
    bool isGif;
    bool flag = false;
    unsigned long startime = 0;
    uint16_t duration = 0;
    int16_t repeat = -1;
    bool hold = false;
    byte pushIcon = 0;
    int16_t iconPosition = 0;
    bool iconWasPushed = false;
    int barData[16] = {0};
    int barSize;
};

Notification notify;

std::vector<std::pair<String, AppCallback>> Apps;

CustomApp *getCustomAppById(String name)
{
    return customApps.count(name) ? &customApps[name] : nullptr;
}

String getAppNameByFunction(AppCallback AppFunction)
{
    for (const auto &appPair : Apps)
    {
        if (appPair.second == AppFunction)
        {
            return appPair.first;
        }
    }

    return ""; // Gibt einen leeren String zurück, wenn die App-Funktion nicht gefunden wurde
}

int findAppIndexByName(const String &name)
{
    auto it = std::find_if(Apps.begin(), Apps.end(), [&name](const std::pair<String, AppCallback> &appPair)
                           { return appPair.first == name; });
    if (it != Apps.end())
    {
        return std::distance(Apps.begin(), it);
    }
    return -1;
}

void TimeApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    if (notify.flag)
        return;
    CURRENT_APP = "Time";
    DisplayManager.getInstance().resetTextColor();
    time_t now = time(nullptr);
    struct tm *timeInfo;
    timeInfo = localtime(&now);
    const char *timeformat = TIME_FORMAT.c_str();
    char t[20];
    char t2[20];
    if (timeformat[2] == ' ')
    {
        strcpy(t2, timeformat);
        if (now % 2)
        {
            t2[2] = ' ';
        }
        else
        {
            t2[2] = ':';
        }
        strftime(t, sizeof(t), t2, localtime(&now));
    }
    else
    {
        strftime(t, sizeof(t), timeformat, localtime(&now));
    }

    DisplayManager.printText(0 + x, 6 + y, t, true, false);

    if (!SHOW_WEEKDAY)
        return;
    int dayOffset = START_ON_MONDAY ? 0 : 1;
    for (int i = 0; i <= 6; i++)
    {
        if (i == (timeInfo->tm_wday + 6 + dayOffset) % 7)
        {
            matrix->drawLine((2 + i * 4) + x, y + 7, (i * 4 + 4) + x, y + 7, matrix->Color(200, 200, 200));
        }
        else
        {
            matrix->drawLine((2 + i * 4) + x, y + 7, (i * 4 + 4) + x, y + 7, matrix->Color(100, 100, 100));
        }
    }
}

void DateApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    if (notify.flag)
        return;
    CURRENT_APP = "Date";
    DisplayManager.getInstance().resetTextColor();
    time_t now = time(nullptr);
    struct tm *timeInfo;
    timeInfo = localtime(&now);
    char d[20];
    strftime(d, sizeof(d), DATE_FORMAT.c_str(), localtime(&now));
    DisplayManager.printText(0 + x, 6 + y, d, true, true);
    if (!SHOW_WEEKDAY)
        return;
    int dayOffset = START_ON_MONDAY ? 0 : 1;
    for (int i = 0; i <= 6; i++)
    {
        if (i == (timeInfo->tm_wday + 6 + dayOffset) % 7)
        {
            matrix->drawLine((2 + i * 4) + x, y + 7, (i * 4 + 4) + x, y + 7, matrix->Color(200, 200, 200));
        }
        else
        {
            matrix->drawLine((2 + i * 4) + x, y + 7, (i * 4 + 4) + x, y + 7, matrix->Color(100, 100, 100));
        }
    }
}

void TempApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    if (notify.flag)
        return;
    CURRENT_APP = "Temperature";
    DisplayManager.getInstance().resetTextColor();
    matrix->drawRGBBitmap(x, y, get_icon(234), 8, 8);
    matrix->setCursor(12 + x, 6 + y);
    if (IS_CELSIUS)
    {
        matrix->print((int)CURRENT_TEMP);
        matrix->print(utf8ascii("°C"));
    }
    else
    {
        int tempF = (CURRENT_TEMP * 9 / 5) + 32;
        matrix->print(tempF);
        matrix->print(utf8ascii("°F"));
    }
}

void HumApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    if (notify.flag)
        return;
    CURRENT_APP = "Humidity";
    DisplayManager.getInstance().resetTextColor();
    matrix->drawRGBBitmap(x, y + 1, get_icon(2075), 8, 8);
    matrix->setCursor(14 + x, 6 + y);
    int humidity = CURRENT_HUM; // Temperatur ohne Nachkommastellen
    matrix->print(humidity);    // Ausgabe der Temperatur
    matrix->print("%");
}

#ifdef ULANZI
void BatApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    if (notify.flag)
        return;
    CURRENT_APP = "Battery";
    DisplayManager.getInstance().resetTextColor();
    matrix->drawRGBBitmap(x, y, get_icon(1486), 8, 8);
    matrix->setCursor(14 + x, 6 + y);
    matrix->print(BATTERY_PERCENT); // Ausgabe des Ladezustands
    matrix->print("%");
}
#endif

void MenuApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state)
{
    if (!MenuManager.inMenu)
        return;
    matrix->fillScreen(0);
    DisplayManager.printText(0, 6, utf8ascii(MenuManager.menutext()).c_str(), true, true);
}

void AlarmApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state)
{
    if (ALARM_ACTIVE)
    {
        matrix->fillScreen(matrix->Color(255, 0, 0));
        CURRENT_APP = "Alarm";
        uint16_t textWidth = getTextWidth("ALARM", false);
        int16_t textX = ((32 - textWidth) / 2);
        matrix->setTextColor(0);
        matrix->setCursor(textX, 6);
        matrix->print("ALARM");
        if (ALARM_SOUND != "")
        {
            if (!PeripheryManager.isPlaying())
                PeripheryManager.playFromFile("/MELODIES/" + ALARM_SOUND + ".txt");
        }
    }
}

void TimerApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state)
{
    if (TIMER_ACTIVE)
    {
        matrix->fillScreen(matrix->Color(0, 255, 0));
        CURRENT_APP = "Timer";
        String menuText = "TIMER";
        uint16_t textWidth = getTextWidth(menuText.c_str(), false);
        int16_t textX = ((32 - textWidth) / 2);
        matrix->setTextColor(0);
        matrix->setCursor(textX, 6);
        matrix->print(menuText);
        if (TIMER_SOUND != "")
        {
            if (!PeripheryManager.isPlaying())
                PeripheryManager.playFromFile("/MELODIES/" + TIMER_SOUND + ".txt");
        }
    }
}

void ShowCustomApp(String name, FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    // Abort if notify.flag is set
    if (notify.flag)
    {
        return;
    }

    // Get custom App by ID
    CustomApp *ca = getCustomAppById(name);

    // Abort if custom App not found
    if (ca == nullptr)
    {
        return;
    }

    // reset custom App properties if last frame
    if (lastFrame)
    {
        ca->iconWasPushed = false;
        ca->scrollposition = 9;
        ca->iconPosition = 0;
        ca->scrollDelay = 0;
    }

    if (!DisplayManager.appIsSwitching)
    {
        if (ca->duration > 0)
        {
            DisplayManager.setAppTime(ca->duration);
        }
    }

    CURRENT_APP = ca->name;
    currentCustomApp = name;

    bool hasIcon = ca->icon;
    uint16_t availableWidth = (hasIcon) ? 24 : 32;

    bool noScrolling = getTextWidth(ca->text.c_str(), false) <= availableWidth;
    if (ca->barSize > 0)
    {
        DisplayManager.drawBarChart(x, y, ca->barData, ca->barSize, hasIcon, ca->color);
    }
    else
    {
        if ((ca->repeat > 0) && (getTextWidth(ca->text.c_str(), false) > availableWidth) && (state->appState == FIXED))
        {
            DisplayManager.setAutoTransition(false);
        }
        else
        {
            DisplayManager.setAutoTransition(true);
        }

        if (getTextWidth(ca->text.c_str(), false) > availableWidth && !(state->appState == IN_TRANSITION))
        {
            if (ca->scrollposition <= -getTextWidth(ca->text.c_str(), false))
            {
                ca->scrollDelay = 0;
                ca->scrollposition = 9;
                if (ca->iconWasPushed && ca->pushIcon == 2)
                {
                    ca->iconWasPushed = false;
                }
                if ((ca->currentRepeat + 1 >= ca->repeat) && (ca->repeat > 0))
                {
                    DisplayManager.setAutoTransition(true);
                    ca->currentRepeat = 0;
                    DisplayManager.nextApp();
                    return;
                }
                else if (ca->repeat > 0)
                {
                    ++ca->currentRepeat;
                }
            }
        }

        if (!noScrolling)
        {
            if ((ca->scrollDelay > MATRIX_FPS * 1.2))
            {
                --ca->scrollposition;
            }
            else
            {
                ++ca->scrollDelay;
                if (hasIcon)
                {
                    if (ca->iconWasPushed && ca->pushIcon == 1)
                    {
                        ca->scrollposition = 0;
                    }
                    else
                    {
                        ca->scrollposition = 9;
                    }
                }
                else
                {
                    ca->scrollposition = 0;
                }
            }
        }
        int16_t textX = (hasIcon) ? ((24 - getTextWidth(ca->text.c_str(), false)) / 2) + 9 : ((32 - getTextWidth(ca->text.c_str(), false)) / 2);
        matrix->setTextColor(ca->color);
        if (noScrolling)
        {
            ca->repeat = -1; // Disable repeat if text is too short for scrolling
            // Display text with rainbow effect if enabled
            if (ca->rainbow)
            {
                DisplayManager.HSVtext(x + textX, 6 + y, ca->text.c_str(), false);
            }
            else
            {
                // Display text
                DisplayManager.printText(x + textX, y + 6, ca->text.c_str(), false, false);
            }
        }
        else
        {
            // Display scrolling text with rainbow effect if enabled
            if (ca->rainbow)
            {
                DisplayManager.HSVtext(x + ca->scrollposition, 6 + y, ca->text.c_str(), false);
            }
            else
            {
                DisplayManager.printText(x + ca->scrollposition, 6 + y, ca->text.c_str(), false, false);
            }
        }
    }

    if (hasIcon)
    {
        // Push icon if enabled and text is scrolling
        if (ca->pushIcon > 0 && !noScrolling && ca->barSize == 0)
        {
            if (ca->iconPosition < 0 && ca->iconWasPushed == false && ca->scrollposition > 8)
            {
                ++ca->iconPosition;
            }

            if (ca->scrollposition < 8 && !ca->iconWasPushed)
            {
                ca->iconPosition = ca->scrollposition - 8;

                if (ca->iconPosition <= -9)
                {
                    ca->iconWasPushed = true;
                }
            }
        }

        // Display animated GIF if enabled and App is fixed, since we have only one gifplayer instance, it looks weird when 2 apps want to draw a different gif
        if (ca->isGif)
        {
            if (state->appState == FIXED)
            {
                DisplayManager.drawGIF(x + ca->iconPosition, y, ca->icon);
            }
        }
        else
        {
            // Display JPEG image
            DisplayManager.drawJPG(x + ca->iconPosition, y, ca->icon);
        }

        // Draw vertical line if text is scrolling
        if (!noScrolling)
        {
            // matrix->drawLine(8 + x + ca->iconPosition, 0 + y, 8 + x + ca->iconPosition, 7 + y, 0);
        }
    }
    // Reset text color
    DisplayManager.getInstance().resetTextColor();
}

void NotifyApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state)
{
    // Check if notification flag is set
    if (!notify.flag)
    {
        return; // Exit function if flag is not set
    }

    // Set current app name
    CURRENT_APP = "Notification";
    // Get current time
    unsigned long currentTime = millis();

    // Check if notification duration has expired or if repeat count is 0 and hold is not enabled
    if ((((currentTime - notify.startime >= notify.duration) && notify.repeat == -1) || notify.repeat == 0) && !notify.hold)
    {
        // Reset notification flags and exit function
        notify.flag = false;
        notify.duration = 0;
        notify.startime = 0;
        notify.scrollposition = 34;
        notify.iconWasPushed = false;
        notify.iconPosition = 0;
        notify.scrollDelay = 0;
        return;
    }

    // Check if notification has an icon
    bool hasIcon = notify.icon;

    // Clear the matrix display
    matrix->fillRect(0, 0, 32, 8, 0);

    // Calculate text and available width
    uint16_t textWidth = getTextWidth(notify.text.c_str(), false);
    uint16_t availableWidth = hasIcon ? 24 : 32;

    // Check if text is scrolling
    bool noScrolling = textWidth <= availableWidth;
    if (notify.barSize > 0)
    {
        DisplayManager.drawBarChart(0, 0, notify.barData, notify.barSize, hasIcon, notify.color);
    }
    else
    {
        // Check if text needs to be scrolled
        if (textWidth > availableWidth && notify.scrollposition <= -textWidth)
        {
            // Reset scroll position and icon position if needed
            notify.scrollDelay = 0;
            notify.scrollposition = 9;

            if (notify.pushIcon == 2)
            {
                notify.iconWasPushed = false;
            }

            if (notify.repeat > 0)
            {
                --notify.repeat;
                if (notify.repeat == 0)
                    return;
            }
        }

        if (!noScrolling)
        {
            if ((notify.scrollDelay > MATRIX_FPS * 1.2))
            {
                --notify.scrollposition;
            }
            else
            {
                ++notify.scrollDelay;
                if (hasIcon)
                {
                    if (notify.iconWasPushed && notify.pushIcon == 1)
                    {
                        notify.scrollposition = 0;
                    }
                    else
                    {
                        notify.scrollposition = 9;
                    }
                }
                else
                {
                    notify.scrollposition = 0;
                }
            }
        }

        // Calculate text X position based on icon presence
        int16_t textX = hasIcon ? ((24 - textWidth) / 2) + 9 : ((32 - textWidth) / 2);

        // Set text color
        matrix->setTextColor(notify.color);

        if (noScrolling)
        {
            // Disable repeat if text is not scrolling
            notify.repeat = -1;

            if (notify.rainbow)
            {
                // Display text in rainbow color if enabled
                DisplayManager.HSVtext(textX, 6, notify.text.c_str(), false);
            }
            else
            {
                // Display text in solid color
                DisplayManager.printText(textX, 6, notify.text.c_str(), false, false);
            }
        }
        else
        {
            if (notify.rainbow)
            {
                // Display scrolling text in rainbow color if enabled
                DisplayManager.HSVtext(notify.scrollposition, 6, notify.text.c_str(), false);
            }
            else
            {
                // Display scrolling text in solid color
                DisplayManager.printText(notify.scrollposition, 6, notify.text.c_str(), false, false);
            }
        }
    }

    // Display icon if present and not pushed
    if (hasIcon)
    {
        // Push icon if enabled and text is scrolling
        if (notify.pushIcon > 0 && !noScrolling && notify.barSize == 0)
        {
            if (notify.iconPosition < 0 && notify.iconWasPushed == false && notify.scrollposition > 8)
            {
                ++notify.iconPosition;
            }

            if (notify.scrollposition < 8 && !notify.iconWasPushed)
            {
                notify.iconPosition = notify.scrollposition - 9;

                if (notify.iconPosition <= -9)
                {
                    notify.iconWasPushed = true;
                }
            }
        }

        // Display animated GIF if
        if (notify.isGif)
        {
            // Display GIF if present
            DisplayManager.drawGIF(notify.iconPosition, 0, notify.icon);
        }
        else
        {
            // Display JPG image if present
            DisplayManager.drawJPG(notify.iconPosition, 0, notify.icon);
        }

        // Display icon divider line if text is scrolling
        if (!noScrolling)
        {
            // matrix->drawLine(8 + notify.iconPosition, 0, 8 + notify.iconPosition, 7, 0);
        }
    }

    // Reset text color after displaying notification
    DisplayManager.getInstance().resetTextColor();
}

// Unattractive to have a function for every customapp wich does the same, but currently still no other option found TODO

void CApp1(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp1);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp2(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp2);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp3(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp3);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp4(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp4);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp5(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp5);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp6(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp6);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp7(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp7);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp8(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp8);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp9(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp9);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp10(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp10);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp11(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp11);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp12(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp12);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp13(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp13);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp14(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp14);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp15(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp15);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp16(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp16);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp17(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp17);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp18(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp18);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp19(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp19);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

void CApp20(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    String name = getAppNameByFunction(CApp20);
    ShowCustomApp(name, matrix, state, x, y, firstFrame, lastFrame);
}

const uint16_t *getWeatherIcon(int code)
{
    switch (code)
    {
    case 1:
        return icon_475;
        break;

    default:
        return icon_475;
        break;
    }
}

void WeatherApp(FastLED_NeoMatrix *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, bool firstFrame, bool lastFrame)
{
    if (notify.flag)
        return;
    CURRENT_APP = "Weather";
    DisplayManager.getInstance().resetTextColor();
    matrix->drawRGBBitmap(x, y, getWeatherIcon(WEATHER_CODE), 8, 8);
    String text = WEATHER_TEMP + "°" + WEATHER_HUM + "%";
    uint16_t textWidth = getTextWidth(text.c_str(), false);
    int16_t textX = ((23 - textWidth) / 2);
    matrix->setCursor(textX + 11, 6 + y);
    matrix->print(utf8ascii(text));
}

void getWeatherData()
{
    Serial.println("UPDATE");
    String weatherUrl = "https://wttr.in/" + CITY + "?format=p1";
    if ((WiFi.status() == WL_CONNECTED))
    {
        HTTPClient http;
        http.begin(weatherUrl);
        http.setTimeout(5000);
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            String payload = http.getString();
            int temperatureIndex = payload.indexOf("temperature_celsius{forecast=\"current\"}");
            int humIndex = payload.indexOf("humidity_percentage{forecast=\"current\"}");
            int weatherCodeIndex = payload.indexOf("weather_code{forecast=\"current\"}");

            if (temperatureIndex >= 0 && weatherCodeIndex >= 0)
            {
                int tempEndIndex = payload.indexOf('\n', temperatureIndex);
                int codeEndIndex = payload.indexOf('\n', weatherCodeIndex);
                int humEndIndex = payload.indexOf('\n', humIndex);
                String temperatureValue = payload.substring(temperatureIndex + 40, tempEndIndex);
                String humValue = payload.substring(humIndex + 40, humEndIndex);
                String weatherCodeValue = payload.substring(weatherCodeIndex + 33, codeEndIndex);

                WEATHER_TEMP = temperatureValue;
                WEATHER_HUM = humValue;
                WEATHER_CODE = weatherCodeValue.toInt();
            }
        }
        http.end();
    }
}

void StartAppUpdater()
{
    // downloader.attach(60,getWeatherData);
    // getWeatherData();
}

OverlayCallback overlays[] = {MenuApp, NotifyApp, AlarmApp, TimerApp};
#endif