/*
* This code is designed for the use with an
* GIDEMA WS2812 RGBShield v1.0 for Arduino Microcontrollers.
*
* It has been tested and worked with Arduino Uno R3 and Arduino Mega 2560.
*
* Restrictions:
* -------------
* GIDEMA WS2812 RGBShield v1.0:
* >> maximum of 4 Amps per LED Port
*
* Arduino Uno R3:
* >> a total of 500 LEDs can be handled, i.e. all ports together. (memory related)
*
* Arduino Mega 2560:
* >> a total of 2400 LEDs can be handled, i.e. all ports together. (memory related)
*
* Ports 1,2 and 4 are connected with 55 LEDs
* Port 3 has 59 LEDs:
*		there are 5 strips but only 4 ports. The first 4 LEDs of the strip on port 3
*		are a virtual 5th strip at the backside to become an equally divided shine at
*		the top plate.
*
* HowTo:
* ------
* LED::setColorToAllLEDs(uint8_t strip, uint32_t color)
* or LED::setColor(uint8_t strip, uint8_t fromLed, uint8_t toLed, uint32_t color)
* or LED::setColor(uint8_t strip, uint8_t led, uint32_t color)
* and LED::show(uint8_t strip)
*
* Created:			19.05.2014
* Last updated:		29.07.2014
* Author:			Florian Probst
* Contact:			florian.probst@gidema.de
* Company:			GIDEMA Solutions GmbH
* Web:				www.gidema.de
*/
#include "Adafruit_NeoPixel.h"
#include "WS2812_Definitions.h"
#include "TimedAction.h" 

#define NUMBER_OF_STRIPS 5
#define PORT1 0
#define PORT2 1
#define PORT3 2
#define PORT4 3

const uint8_t port1_number_of_leds = 240;  // 0 is not allowed
const uint8_t port2_number_of_leds = 5; // 0 is not allowed
const uint8_t port3_number_of_leds = 5; // 0 is not allowed
const uint8_t port4_number_of_leds = 5; // 0 is not allowed

const uint8_t port1_brightness = 20;
const uint8_t port2_brightness = 25;
const uint8_t port3_brightness = 25;
const uint8_t port4_brightness = 25;

const uint32_t color1 = DEEPSKYBLUE; //LIME;
const uint32_t color2 = WHITE;
const uint32_t color3 = 0x006E00;
const uint32_t color4 = 0x004800;
const uint32_t color5 = 0x001F00;

const uint8_t fxSpeed = 10; //faster than 50 will cause light-bugs (seen at 50 leds on one strip)
const uint8_t fxLimit = 240; //the effect runs to light nr.

/*const uint32_t color2 = WHITE;
const uint32_t color3 = WHITE;
const uint32_t color4 = WHITE;
const uint32_t color5 = WHITE;*/

const uint8_t port1_pin = 2;
const uint8_t port2_pin = 4;
const uint8_t port3_pin = 7;
const uint8_t port4_pin = 12;
const uint8_t endstop_opened_pin = 31;
const uint8_t endstop_closed_pin = 33;

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(port1_number_of_leds, port1_pin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(port2_number_of_leds, port2_pin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(port3_number_of_leds, port3_pin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(port4_number_of_leds, port4_pin, NEO_GRB + NEO_KHZ800);

TimedAction fxKnightRiderAction = TimedAction(fxSpeed, fxKnightRider);

bool fxKnightRiderInitialized[4];
uint8_t fxKnightRiderDirection[4];
uint16_t fxKnightRiderPos[4];
bool fxKnightRiderEnabled[4];
bool endstop_processed = false;


namespace LED {
	typedef struct {
		uint8_t pin;
		uint16_t leds;
		uint8_t brightness;
		Adafruit_NeoPixel *neopixel;
	} Strip;

	Strip strips[4];

	void setColorToAllLEDs(uint8_t strip, uint32_t color) {
		for (int i = 0; i < strips[strip].leds; i++) {
			strips[strip].neopixel->setPixelColor(i, color);
		}
	}

	void setColor(uint8_t strip, uint8_t fromLed, uint8_t toLed, uint32_t color) {
		for (int i = fromLed; i <= toLed; i++) {
			strips[strip].neopixel->setPixelColor(i, color);
		}
	}

	void setColor(uint8_t strip, uint8_t led, uint32_t color) {
		strips[strip].neopixel->setPixelColor(led, color);
	}

	void show(uint8_t strip) {
		strips[strip].neopixel->show();
	}

	void initializeStrip(uint8_t strip) {
		strips[strip].neopixel->begin();
		strips[strip].neopixel->setBrightness(strips[strip].brightness);
		setColorToAllLEDs(strip, BLACK);
		strips[strip].neopixel->show();
	}

	uint16_t numLED(uint8_t strip) {
		return strips[strip].leds;
	}
}


void setup()
{
	Serial.begin(115200);
	Serial.println(F("System starting..."));
	delay(2000);

	LED::strips[PORT1].pin = port1_pin;
	LED::strips[PORT1].leds = port1_number_of_leds;
	LED::strips[PORT1].brightness = port1_brightness;
	LED::strips[PORT1].neopixel = &strip1;
	LED::strips[PORT2].pin = port2_pin;
	LED::strips[PORT2].leds = port2_number_of_leds;
	LED::strips[PORT2].brightness = port2_brightness;
	LED::strips[PORT2].neopixel = &strip2;
	LED::strips[PORT3].pin = port3_pin;
	LED::strips[PORT3].leds = port3_number_of_leds;
	LED::strips[PORT3].brightness = port3_brightness;
	LED::strips[PORT3].neopixel = &strip3;
	LED::strips[PORT4].pin = port4_pin;
	LED::strips[PORT4].leds = port4_number_of_leds;
	LED::strips[PORT4].brightness = port4_brightness;
	LED::strips[PORT4].neopixel = &strip4;

	Serial.println(F("initialize LED strips..."));
	int leds = port1_number_of_leds + port2_number_of_leds + port3_number_of_leds + port4_number_of_leds;
	Serial.print(F("Number of LEDs configured: "));
	Serial.println(leds);

	//delay(2000);

	LED::initializeStrip(PORT1);
	LED::initializeStrip(PORT2);
	LED::initializeStrip(PORT3);
	LED::initializeStrip(PORT4);

	fxKnightRiderEnabled[PORT1] = true;
	fxKnightRiderEnabled[PORT2] = true;
	fxKnightRiderEnabled[PORT3] = true;
	fxKnightRiderEnabled[PORT4] = true;

	Serial.print(F("System started. Free SRAM: "));
	Serial.println(freeMemory());

	//configure endstops
	pinMode(endstop_closed_pin, INPUT);
	pinMode(endstop_opened_pin, INPUT);
	delay(1000);
	initFxKnightRider();
}

void initFxKnightRider() {
	//initialize all strips for knightrider effect
	for (int strip = 0; strip < 4; strip++) {
		if (fxKnightRiderEnabled[strip]) {
			if (!fxKnightRiderInitialized[strip]) {
				LED::setColorToAllLEDs(strip, color1);		//set all leds to base color
				LED::show(strip);
				fxKnightRiderPos[strip] = 0;				//pointer to first led
				fxKnightRiderDirection[strip] = 1;			//from first to last led
				fxKnightRiderInitialized[strip] = true;		//only run this once
			}
		}
	}
}

void loop()
{
	if (endstop_processed) {
		LED::setColorToAllLEDs(PORT1, color1);
		LED::show(PORT1);
		initFxKnightRider();
		endstop_processed = false;
	}
	
	while (digitalRead(endstop_closed_pin) == true) {
		if (!endstop_processed) {
			LED::setColorToAllLEDs(PORT1, RED);
			LED::show(PORT1);
			endstop_processed = true;
			delay(200);
		}
	}

	while (digitalRead(endstop_opened_pin) == true) {
		if (!endstop_processed) {
			LED::setColorToAllLEDs(PORT1, GREEN); 
			LED::show(PORT1);
			endstop_processed = true;
			delay(200);
		}
	}
	fxKnightRiderAction.check();
	//Serial.println(millis());
	//delay(1000);
}

void fxKnightRider() {
	for (int strip = 0; strip < 4; strip++) {
		if (fxKnightRiderDirection[strip] == 1) {			//from first to last led
			int p = fxKnightRiderPos[strip];
			if (p <= fxLimit) {
				/*if (strip == 2) { // which will be our port 3 strip, that has 4 additional LEDs at the back side (first 4 leds)
								  //virtual 5th strip (back side) hack
					LED::setColor(strip, p + 4 - 7, p + 4, color2);
					LED::setColor(strip, p + 4 - 8, color1);
					if (p >= 53) LED::setColor(strip, 0, 3, color2);
					if (p == 52) LED::setColor(strip, 0, 2, color2);
					//if(p <= 52) LED::setColor(strip, 0, 1, color2);
					if (p <= 51) LED::setColor(strip, 0, 0, color2);
					if (p <= 45) LED::setColor(strip, 0, 3, color1); //show green.
				}
				else {*/
					LED::setColor(strip, p - 7, p, color2);
					LED::setColor(strip, p - 8, color1);
				//}
			}
			fxKnightRiderPos[strip]++;
			if (fxKnightRiderPos[strip] == fxLimit) { 
				fxKnightRiderPos[strip] = fxLimit - 1;
				fxKnightRiderDirection[strip] = 0; //change direction from last to first led
			}
		}
		if (fxKnightRiderDirection[strip] == 0) {			//from last to first led
			int p = fxKnightRiderPos[strip];
			/*if (strip == 2) { // which will be our port 3 strip, that has 4 additional LEDs at the back side (first 4 leds)
							  //virtual 5th strip (back side) hack
				LED::setColor(strip, p + 4, p + 4 + 7, color2);
				LED::setColor(strip, p + 4 + 8, color1);
				if (p == 47) LED::setColor(strip, 3, 3, color1);
				if (p == 46) LED::setColor(strip, 2, 3, color1);
				if (p == 45) LED::setColor(strip, 1, 3, color1);
				if (p <= 43) LED::setColor(strip, 0, 3, color1);

			}
			else {*/
				LED::setColor(strip, p, p + 7, color2);
				LED::setColor(strip, p + 8, color1);
			//}
			fxKnightRiderPos[strip]--;
			if (fxKnightRiderPos[strip] == -1) { // this constant is not good, but fastest way to get it work. All strips have 55 led's
				fxKnightRiderPos[strip]++;
				fxKnightRiderDirection[strip] = 1; //change direction from last to first led
			}
		}
	}

	if (fxKnightRiderEnabled[PORT1]) LED::show(PORT1);
	if (fxKnightRiderEnabled[PORT2]) LED::show(PORT2);
	if (fxKnightRiderEnabled[PORT3]) LED::show(PORT3);
	if (fxKnightRiderEnabled[PORT4]) LED::show(PORT4);
}

uint16_t freeMemory() {
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}