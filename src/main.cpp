#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "wifi_info.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

#define DHTPIN  5
#define DHTTYPE DHT21

//==============================
// HomeKit setup and loop
//==============================

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_temperature;
extern "C" homekit_characteristic_t cha_humidity;

DHT_Unified dht(DHTPIN, DHTTYPE);

// Called when the value is read by iOS Home APP
homekit_value_t cha_programmable_switch_event_getter() {
	// Should always return "null" for reading, see HAP section 9.75
	return HOMEKIT_NULL_CPP();
}

void my_homekit_setup() {
	dht.begin();
	arduino_homekit_setup(&config);
}

int random_value(int min, int max) {
	return min + random(max - min);
}

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

void my_homekit_report() {
	
	sensors_event_t event;
	dht.temperature().getEvent(&event);

	float t = 0;
	float h = 0;

	if (!isnan(event.temperature)) {
		t = event.temperature;	
	}

	dht.humidity().getEvent(&event);

	if (!isnan(event.relative_humidity)) {
		h = event.relative_humidity;
	}

	cha_temperature.value.float_value = t;
	homekit_characteristic_notify(&cha_temperature, cha_temperature.value);

	cha_humidity.value.float_value = h;
	homekit_characteristic_notify(&cha_humidity, cha_humidity.value);

	LOG_D("t %.1f, h %.1f", (uint8_t)t, h);
}

void my_homekit_loop() {
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_report_millis) {
		// report sensor values every 10 seconds
		next_report_millis = t + 10 * 1000;
		my_homekit_report();
	}
	if (t > next_heap_millis) {
		// Show heap info every 5 seconds
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}


void setup() {
	Serial.begin(115200);
	wifi_connect(); // in wifi_info.h
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
	my_homekit_setup();
}

void loop() {
	my_homekit_loop();
	delay(10);
}