# ESP32 A2DP audio receiver
This is a project for an ESP32 based A2DP bluetooth receiver. It uses the I2S interface to stream digital audio to an external DAC.
![Receiver box](https://github.com/diglooo/esp-bt-sink/blob/main/pictures/main.PNG)
Features:
 - Simple PlatformIO project
 - Based on the A2DP library from [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)
 - Standard SBC codec. 
	 - Supporting AptX and AAC [is possible](https://github.com/cfint/esp32-a2dp-sink) but that requires vast changes to the project and has a low impact on sound quality.
 - PCM5102A I2S DAC (buy on Amazon)
 - ESP32-WROOM-32 in a Devkitc-V4 (buy on Amazon)
 - User button with RGB LED (WS2812b) for secure pairing and status.
 - Power amplifier control: turn off a relay when nobody is connected or when playback is inacive for too long.
 - 3D printable enclosure made with PTC Creo Parametric 7 (STEP files available)
