#include <FastLED.h>

#define WITH_LCD 1
// See Library "Docs" folder for possible commands etc.
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>


#define LCD_CHARS   16
#define LCD_LINES    2


// How many leds are in the strip?
#define NUM_LEDS 16

// Data pin that led data will be written out over
#define DATA_PIN 2

#define LED_TYPE    WS2812


// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#define NUM_MODES 2
uint8_t nOptions[] = { 4, 3 };

uint8_t currentOption;

ClickEncoder *encoder;
uint8_t mode = 0;
uint8_t clicked = 0;
uint8_t dbclicked = 0;
long timeOut;
uint8_t interval = 80;
uint8_t hue;
uint8_t sat = 255;
uint8_t bright = 100;
uint8_t increment;





void timerIsr() {
  encoder->service();
}



void setup()
{

	// initialize LEDS
	LEDS.setBrightness(100);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	
	// initialize library
	lcd.begin(LCD_CHARS, LCD_LINES);
	encoder = new ClickEncoder(A1, A0, A2, 4);
	encoder->setAccelerationEnabled(true);
	Timer1.initialize(1000);
	Timer1.attachInterrupt(timerIsr);
	
	timeOut = millis(); // initilaze timout
	
	// initialize display
	lcd.backlight();
	display(0);
	delay(1000);
}


void loop()
{
	increment = encoder->getValue();

	display(increment);

	if (millis() > timeOut) {
		timeOut = millis() + interval;


		switch (mode) {
			case 0:
				confetti();
				break;
			case 1:
				fill_solid(leds, NUM_LEDS, CHSV(hue, sat, bright));
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
			break;
		case ClickEncoder::DoubleClicked:
			dbclicked++;
			currentOption = 0;
			clicked = 0;
			mode = dbclicked % NUM_MODES;
			display(0);
			break;
		case ClickEncoder::Held:
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
			lcd.setCursor(0, 1);
			lcd.print("confetti");
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
			lcd.setCursor(0, 1);
			lcd.print("solid   ");
			break;
	}
	// format/print second line of display
	lcd.setCursor(10, 1);
	lcd.print(currentOption + 1);
	lcd.print(" of ");
	lcd.print(nOptions[mode]);
}

void confetti()
{
	// random colored speckles that blink in and fade smoothly
	fadeToBlackBy(leds, NUM_LEDS, 35);//long strip used five
	int pos = random16(NUM_LEDS);
	leds[pos] += CHSV(hue + random8(64), sat, bright);
}


void adjustHue()
{
	lcd.setCursor(0, 0);
	lcd.print("Hue=");
	lcd.setCursor(4, 0);
	lcd.print(hue);
	lcd.print("         ");
}

void adjustSaturation()
{
	lcd.setCursor(0, 0);
	lcd.print("Saturation=");
	lcd.setCursor(11, 0);
	lcd.print(sat);
	lcd.print("  ");
}

void adjustBrightness()
{
	lcd.setCursor(0, 0);
	lcd.print("Brightness=");
	lcd.setCursor(11, 0);
	lcd.print(bright);
	lcd.print("  ");
}

void adjustInterval()
{
	lcd.setCursor(0, 0);
	lcd.print("Interval=");
	lcd.setCursor(9, 0);
	lcd.print(interval);
	lcd.print("    ");
}
