#include "AudioTools.h"
#include "BluetoothA2DPSinkQueued.h"
#include "soc/rtc_wdt.h"

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

#define PIN_BCK 26
#define PIN_DATA 27
#define PIN_WS 25
#define PIN_RELAY 14

int relay_status = 0;

void avrc_metadata_callback(uint8_t id, const uint8_t *text)
{
    Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
    if (id == ESP_AVRC_MD_ATTR_PLAYING_TIME)
    {
        uint32_t playtime = String((char *)text).toInt();
        Serial.printf("==> Playing time is %d ms (%d seconds)\n", playtime, (int)round(playtime / 1000.0));
    }
}

void setup()
{
    pinMode(PIN_RELAY, OUTPUT);
    audio_tools::I2SConfig cfg = i2s.defaultConfig();
    cfg.pin_bck = PIN_BCK;
    cfg.pin_data = PIN_DATA;
    cfg.pin_ws = PIN_WS;
    i2s.begin(cfg);
    a2dp_sink.set_avrc_metadata_attribute_mask(ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM | ESP_AVRC_MD_ATTR_PLAYING_TIME);
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.start("Cuccumella");

    Serial.begin(115200);
    Serial.println("Started");
}

int connected = 0, connected_old = 0;
int cnt = 0;
void loop()
{
    connected_old = connected;
    connected = a2dp_sink.is_connected();

    if (connected && !connected_old)
    {
        Serial.print(a2dp_sink.get_peer_name());
        Serial.println(" connected");
        digitalWrite(PIN_RELAY, 1);
    }

    if (!connected && connected_old)
    {
        Serial.println("Disconnected");
        digitalWrite(PIN_RELAY, 0);
    }

    delay(100);
}