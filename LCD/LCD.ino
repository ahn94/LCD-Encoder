#include <FastLED.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
// test test test lcd presets

#define LCD_CHARS   20		// columns
#define LCD_LINES    4		// rows
#define NUM_LEDS 147		// number of leds
#define DATA_PIN 2	
// led data pin
#define LED_TYPE    WS2812B	// led type


// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

ClickEncoder *encoder;	// pointer to encoder
uint8_t increment;		// encoder output stored here every loop

// general menu variables
#define NUM_MODES 4					// number of modes;
uint8_t nOptions[] = { 4, 3, 3 , 1};	// options per mode;
uint8_t currentOption;				// option for current mode
uint8_t mode = 3;					// current mode
uint8_t clicked = 0;				// used to % = cycle through options for current mode
uint8_t dbclicked = 0;				// used to % = cycle through modes
long timeOut = -1;					// timout

// general color settings
uint8_t hue = 87;			// hue or color 
uint8_t sat = 255;		// saturation
uint8_t bright = 180;	// brightness

// loop delay
uint8_t interval = 70;	// timing speed for animation

// rainbow settings
uint8_t deltaHue = 3;	// DeltaHue for rainbow adjustment
uint8_t rHue = 87;		// rainbow rotating hue

// backlight toggle
bool lightOn = true;






void timerIsr() {
  encoder->service();
}



void setup()
{
	// initialize LEDS
	LEDS.setBrightness(255);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	
	// initialize encoder & timer
	encoder = new ClickEncoder(A1, A0, A2, 4);
	encoder->setAccelerationEnabled(true);
	Timer1.initialize(1000);
	Timer1.attachInterrupt(timerIsr);
	timeOut = millis(); // initilaze timout
	
	// initialize display
	lcd.begin(LCD_CHARS, LCD_LINES);
	lcd.backlight();
	display(0);
	delay(1000);
}


void loop()
{
	increment = encoder->getValue();
	if (increment != 0) {
		display(increment);
	}

	if (millis() > timeOut) {
		timeOut = millis() + interval;
		switch (mode) {
			case 0:
				FastLED.delay(interval);
				confetti();
				break;
			case 1:
				fill_solid(leds, NUM_LEDS, CHSV(hue, sat, bright));
				break;
			case 2:
				fill_rainbow(leds, NUM_LEDS, rHue, deltaHue);
				FastLED.setBrightness(bright);
				rHue += 2;
				break;
			case 3:
				fill_solid(leds, NUM_LEDS, CHSV(rHue, sat, bright));
				rHue += 2;
				break;
		}
	}
	FastLED.show();

	
	ClickEncoder::Button b = encoder->getButton();

	if (b != ClickEncoder::Open) {
		switch (b) {
		case ClickEncoder::Clicked:
			clicked++;
			currentOption = clicked % nOptions[mode];
			display(0);
			break;
		case ClickEncoder::DoubleClicked:
			dbclicked++;
			currentOption = 0;
			clicked = 0;
			FastLED.setBrightness(255);
			mode = dbclicked % NUM_MODES;
			display(0);
			break;
		case ClickEncoder::Held:
			if (lightOn) {
				lcd.noBacklight();
				lightOn = false;
				delay(500);
			} 
			else {
				lcd.backlight();
				lightOn = true;
				delay(500);
			}
			break;
		}
	}

}

void display(uint8_t incr)
{
	switch (mode) // mode displayer switch
	{
		case 0: // confetti mode
			switch (currentOption) // Options displayer switch
			{
				case 0: // color option
					hue += incr;
					adjustHue(); // print color display
					break;
				case 1: // saturation option
					sat += incr; 
					adjustSaturation(); // print saturation display
					break;
				case 2: // brightness option
					bright += incr;
					adjustBrightness(); // print brightness display 
					break;
				case 3: // interval option
					interval += incr;
					adjustInterval(); // print interval option
					break;
			}
			lcd.setCursor(0, 0);
			lcd.print("      CONFETTI      ");
			break;
		case 1: // solid color mode
			switch (currentOption) // Options displayer switch
			{
				case 0: // color option
					hue += incr;
					adjustHue(); // print color display
					break;
				case 1: // saturation option
					sat += incr;
					adjustSaturation(); // print saturation display
					break;
				case 2: // brightness option
					bright += incr;
					adjustBrightness(); // print brightness display 
					break;
			}
			lcd.setCursor(0, 0);
			lcd.print("       SOLID       ");
			break;
		case 2: // rainbow mode
			switch (currentOption)
			{
				case 0: // interval option
					interval += incr;
					adjustInterval(); // print interval option
					break;
				case 1: // huedelta option
					deltaHue += incr;
					adjustDeltaHue(); // print saturation display
					break;
				case 2: // brightness option
					bright += incr;
					adjustBrightness(); // print brightness display 
					break;
			
			}
			lcd.setCursor(0, 0);
			lcd.print("      RAINBOW      ");
			break;
		case 3: // rainbow pulse
			interval += incr;
			adjustInterval();
			lcd.setCursor(0, 0);
			lcd.print("       PULSE        ");
			break;
	}
	// format/print second line of display
	lcd.setCursor(0, 2);
	lcd.print("OPTION |");
	lcd.setCursor(0, 3);

	lcd.print(currentOption + 1);
	lcd.print(" of ");
	lcd.print(nOptions[mode]);
	lcd.print(" | ");
	lcd.setCursor(0, 1);
	lcd.print("--------------------");
}

void confetti()
{
	// random colored speckles that blink in and fade smoothly
	fadeToBlackBy(leds, NUM_LEDS, 5);//long strip used five
	leds[random16(NUM_LEDS)] += CHSV(hue + random8(64), sat, bright);
	leds[random16(NUM_LEDS)] += CHSV(hue + random8(64), sat, bright);

}

void adjustHue()
{
	lcd.setCursor(9, 2);
	lcd.print("COLOR     ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(hue);
	lcd.print("]    ");
}

void adjustSaturation()
{
	lcd.setCursor(9, 2);
	lcd.print("SATURATION   ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(sat);
	lcd.print("]    ");
}

void adjustBrightness()
{
	lcd.setCursor(9, 2);
	lcd.print("BRIGHTNESS     ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(bright);
	lcd.print("]    ");
}

void adjustInterval()
{
	lcd.setCursor(9, 2);
	lcd.print("SPEED       ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(interval);
	lcd.print("]    ");
}

void adjustDeltaHue()
{
	lcd.setCursor(9, 2);
	lcd.print("DELTAHUE      ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(deltaHue);
	lcd.print("]    ");
}