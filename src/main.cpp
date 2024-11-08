#include "AudioTools.h"
#include "BluetoothA2DPSinkQueued.h"
#include <Bonezegei_WS2812.h>

// On PCM5102A board, you must bridge SCK on PCB
#define PIN_BCK 26  // PCM5102A BCK
#define PIN_DATA 27 // PCM5102A DIN
#define PIN_WS 25   // PCM5102A LCK
#define PIN_RELAY 14
#define PIN_PAIR_BUTTON 13
#define PIN_LED_SERIAL 15
#define BUTTON_PRESSED_THRESH 40 // touch limit
#define NO_SOUND_TIMEOUT 30000

#define APP_STA_INIT 0
#define APP_STA_DISCONNECTED 1
#define APP_STA_PAIRING 2
#define APP_STA_CHECK_PAIRING 3
#define APP_STA_PAIRING_FAILED 4
#define APP_STA_CONNECTED 5
#define LED_QUANTITY 2

// param WS2812 pin
Bonezegei_WS2812 rgb_led(PIN_LED_SERIAL, LED_QUANTITY);
int relay_status = 0;
int connected = 0, connected_old = 0;
int cnt = 0;
int pairing_millis = 0;
long user_button_millis = 30000;
long last_play_millis = 0;
long last_blink_millis = 0;
uint32_t blink_led_color = 0;
uint32_t static_led_color = 0;
int blink_var = 0;
int user_ack = 0;
int APP_STATE = APP_STA_INIT;

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void avrc_metadata_callback(uint8_t id, const uint8_t *text)
{
    Serial.printf("AVRC attribute id 0x%x = %s\n", id, text);
}

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
    if (state == ESP_A2D_AUDIO_STATE_STARTED)
    {
        last_play_millis = millis();
    }
}

void setup()
{
    pinMode(PIN_PAIR_BUTTON, INPUT_PULLUP);
    pinMode(PIN_RELAY, OUTPUT);
}

void led_control()
{
    if (millis() - last_blink_millis > 500)
    {
        last_blink_millis = millis();
        if (blink_led_color)
        {
            if (blink_var)
                rgb_led.fill(blink_led_color);
            else
                rgb_led.fill(0x00000000);
            blink_var = !blink_var;
        }
        else
        {
            blink_var = 1;
        }
    }

    if (static_led_color)
        rgb_led.fill(static_led_color);

    if (!static_led_color && !blink_led_color)
        rgb_led.fill(0x00000000);
}

void loop()
{
    connected_old = connected;
    connected = a2dp_sink.is_connected();

    if (!digitalRead(PIN_PAIR_BUTTON))
    {
        user_button_millis = millis();
    }
    user_ack = (millis() - user_button_millis < 15000);

    switch (APP_STATE)
    {
    case APP_STA_INIT:
    {
        Serial.begin(115200);
        rgb_led.begin();
        rgb_led.fill(0x000000);
        Serial.println("APP_STA_INIT");
        pinMode(PIN_RELAY, OUTPUT);
        pinMode(PIN_PAIR_BUTTON, INPUT_PULLUP);
        audio_tools::I2SConfig cfg = i2s.defaultConfig();
        cfg.pin_bck = PIN_BCK;
        cfg.pin_data = PIN_DATA;
        cfg.pin_ws = PIN_WS;
        i2s.begin(cfg);
        a2dp_sink.set_avrc_metadata_attribute_mask(ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM);
        a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
        a2dp_sink.set_on_audio_state_changed(audio_state_changed);
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
             user_button_millis = millis()+ 40000;
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
         user_button_millis = millis()+ 40000;
        APP_STATE = APP_STA_DISCONNECTED;
        break;
    }

    case APP_STA_CONNECTED:
    {
        Serial.println("APP_STA_CONNECTED");
        digitalWrite(PIN_RELAY, 1);

        /*    if (millis() - last_play_millis < NO_SOUND_TIMEOUT)
            {
                digitalWrite(PIN_RELAY, 1);
            }
            else
            {
                digitalWrite(PIN_RELAY, 0);
            }
    */
        if (!connected)
        {
            APP_STATE = APP_STA_DISCONNECTED;
            user_button_millis = millis()+ 40000;
        }
        break;
    }
    }

    if (user_ack)
    {
        static_led_color = 0;
        blink_led_color = 0xffffff;
    }

    if (!user_ack && APP_STATE == APP_STA_DISCONNECTED)
    {
        static_led_color = 0;
        blink_led_color = 0;
    }

    if (APP_STATE == APP_STA_CONNECTED)
    {
        static_led_color = 0xa0fff0f;
        blink_led_color = 0;
    }

    led_control();
    delay(100);
}