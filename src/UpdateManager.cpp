#include <UpdateManager.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"
#include "DisplayManager.h"
#include <Ticker.h>
#include "Globals.h"

#define URL_fw_Version "https://raw.githubusercontent.com/Blueforcer/awtrix-light/main/version"
#define URL_fw_Bin "https://raw.githubusercontent.com/Blueforcer/awtrix-light/main/docs/flasher/firmware/firmware.bin"

Ticker UpdateTicker;

// The getter for the instantiated singleton instance
UpdateManager_ &UpdateManager_::getInstance()
{
    static UpdateManager_ instance;
    return instance;
}

// Initialize the global shared instance
UpdateManager_ &UpdateManager = UpdateManager.getInstance();

void update_started()
{
}

void update_finished()
{
}

void update_progress(int cur, int total)
{
    DisplayManager.drawProgressBar(cur, total);
}

void update_error(int err)
{
    DisplayManager.clear();
    DisplayManager.printText(0, 6, "FAIL", true, true);
    DisplayManager.show();
}

void UpdateManager_::updateFirmware()
{
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);

    httpUpdate.onStart(update_started);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onError(update_error);

    t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
}

bool UpdateManager_::checkUpdate(bool withScreen)
{
    if (withScreen)
    {
        DisplayManager.clear();
        DisplayManager.printText(0, 6, "CHECK", true, true);
        DisplayManager.show();
    }

    String payload;
    int httpCode;
    String fwurl = "";
    fwurl += URL_fw_Version;
    fwurl += "?";
    fwurl += String(rand());
    Serial.println(fwurl);
    WiFiClientSecure *client = new WiFiClientSecure;

    if (client)
    {
        client->setCACert(rootCACertificate);
        HTTPClient https;

        if (https.begin(*client, fwurl))
        { // HTTPS
            Serial.print("[HTTPS] GET...\n");
            // start connection and send HTTP header
            delay(100);
            httpCode = https.GET();
            delay(100);
            if (httpCode == HTTP_CODE_OK) // if version received
            {
                payload = https.getString(); // save received version
            }
            else
            {
                Serial.print("error in downloading version file:");
                Serial.println(httpCode);
            }
            https.end();
        }
        delete client;
    }

    if (httpCode == HTTP_CODE_OK) // if version received
    {
        payload.trim();
        if (payload.equals(VERSION))
        {
            UPDATE_AVAILABLE = false;
            Serial.printf("\nDevice already on latest firmware version: %s\n", VERSION);
            if (withScreen)
            {
                DisplayManager.clear();
                DisplayManager.printText(0, 6, "NO UP :(", true, true);
                DisplayManager.show();
                delay(1000);
            }
            return 0;
        }
        else
        {
            UPDATE_AVAILABLE = true;
            return 1;
        }
    }
    UPDATE_AVAILABLE = false;
    return 0;
}

void checkUpdateNoReturn()
{
    Serial.println("Check Update");
    UpdateManager.getInstance().checkUpdate(false);
}

void UpdateManager_::setup()
{
    UpdateTicker.attach(3600, checkUpdateNoReturn);
}
