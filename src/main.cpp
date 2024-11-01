#include "AudioTools.h"
#include "BluetoothA2DPSinkQueued.h"

// On PCM5102A board, you must bridge SCK on PCB
#define PIN_BCK 26  // PCM5102A BCK
#define PIN_DATA 27 // PCM5102A DIN
#define PIN_WS 25   // PCM5102A LCK
#define PIN_RELAY 14
#define PIN_TOUCH_BUTTON 13
#define BUTTON_PRESSED_THRESH 40 // touch limit

#define APP_STA_INIT 0
#define APP_STA_DISCONNECTED 1
#define APP_STA_PAIRING 2
#define APP_STA_CHECK_PAIRING 3
#define APP_STA_PAIRING_FAILED 4
#define APP_STA_CONNECTED 5

int relay_status = 0;
int connected = 0, connected_old = 0;
int cnt = 0;
int pairing_millis = 0;
int user_button_millis = 30000;
int user_ack = 0;
int APP_STATE = APP_STA_INIT;

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void avrc_metadata_callback(uint8_t id, const uint8_t *text)
{
    Serial.printf("AVRC attribute id 0x%x = %s\n", id, text);
}

void setup()
{
}

void loop()
{

    connected_old = connected;
    connected = a2dp_sink.is_connected();

    if (digitalRead(PIN_TOUCH_BUTTON))
    {
        user_button_millis = millis();
    }
    user_ack = (millis() - user_button_millis < 15000);

    switch (APP_STATE)
    {
    case APP_STA_INIT:
    {
        Serial.begin(115200);
        Serial.println("APP_STA_INIT");
        pinMode(PIN_RELAY, OUTPUT);
        pinMode(PIN_TOUCH_BUTTON, INPUT_PULLUP);
        audio_tools::I2SConfig cfg = i2s.defaultConfig();
        cfg.pin_bck = PIN_BCK;
        cfg.pin_data = PIN_DATA;
        cfg.pin_ws = PIN_WS;
        i2s.begin(cfg);
        a2dp_sink.set_avrc_metadata_attribute_mask(ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM);
        a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
        a2dp_sink.activate_pin_code(true);
        a2dp_sink.start("Cuccumella");
        APP_STATE = APP_STA_DISCONNECTED;
        break;
    }

    case APP_STA_DISCONNECTED:
    {
        Serial.println("APP_STA_DISCONNECTED");
        digitalWrite(PIN_RELAY, 0);
        if (a2dp_sink.pin_code())
        {
            APP_STATE = APP_STA_PAIRING;
        }
        if (connected)
        {
            APP_STATE = APP_STA_CONNECTED;
            pairing_millis = 0;
        }
        break;
    }

    case APP_STA_PAIRING:
    {
        Serial.println("APP_STA_PAIRING");
        if (user_ack)
        {
            a2dp_sink.confirm_pin_code();
            APP_STATE = APP_STA_CHECK_PAIRING;
        }
        if (a2dp_sink.pin_code() == 0)
        {
            APP_STATE = APP_STA_DISCONNECTED;
        }
        break;
    }

    case APP_STA_CHECK_PAIRING:
    {
        Serial.println("APP_STA_CHECK_PAIRING");
        if (connected)
        {
            APP_STATE = APP_STA_CONNECTED;
            pairing_millis = 0;
        }
        if (pairing_millis++ > 10 || a2dp_sink.pin_code() == 0)
        {
            APP_STATE = APP_STA_PAIRING_FAILED;
        }
        break;
    }

    case APP_STA_PAIRING_FAILED:
    {
        Serial.println("APP_STA_PAIRING_FAILED");
        APP_STATE = APP_STA_DISCONNECTED;
        break;
    }

    case APP_STA_CONNECTED:
    {
        Serial.println("APP_STA_CONNECTED");
        digitalWrite(PIN_RELAY, 1);

        if (!connected)
        {
            APP_STATE = APP_STA_DISCONNECTED;
        }
        break;
    }
    }

    delay(100);
}