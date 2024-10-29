#include "AudioTools.h"
#include "BluetoothA2DPSinkQueued.h"
#include "soc/rtc_wdt.h"

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

// for esp_a2d_connection_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv426esp_a2d_connection_state_t
void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
    Serial.println(a2dp_sink.to_str(state));
}

// for esp_a2d_audio_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv421esp_a2d_audio_state_t
void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
    Serial.println(a2dp_sink.to_str(state));
}

void setup()
{
    //rtc_wdt_protect_off();
    //rtc_wdt_disable();

    auto cfg = i2s.defaultConfig();
    cfg.pin_bck = 26;
    cfg.pin_data = 27;
    cfg.pin_ws = 25;
    i2s.begin(cfg);

    a2dp_sink.set_on_connection_state_changed(connection_state_changed);
    a2dp_sink.set_on_audio_state_changed(audio_state_changed);
    a2dp_sink.start("Cuccumella");
}

void loop()
{
    //rtc_wdt_feed();
    //vTaskDelay(pdMS_TO_TICKS(100));
    delay(100);
}