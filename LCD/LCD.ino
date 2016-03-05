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
uint8_t last, value;
uint8_t mode = 0;
uint8_t CorB = 0;
uint8_t clicked = 0;
uint8_t dbclicked = 0;
long timeOut;
uint8_t interval = 80;
uint16_t lastInteveral = 80;
uint8_t color;



void timerIsr() {
  encoder->service();
}



void setup()
{
	Serial.begin(115200);
	LEDS.setBrightness(100);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	// initialize library
	lcd.begin(LCD_CHARS, LCD_LINES);
	encoder = new ClickEncoder(A1, A0, A2, 4);
	encoder->setAccelerationEnabled(true);
	Timer1.initialize(1000);
	Timer1.attachInterrupt(timerIsr);

	// initialize timout
	last = -1;
	timeOut = millis();

	// intitialize last delay to current so doesn't run till it changes
	lastInteveral = interval;

	lcd.backlight();
	// print starting screen
	lcd.setCursor(0, 0);
	// starts with value most used setting
	lcd.print("color=");
	lcd.setCursor(14, 0);
	// print mode in top right
	lcd.print('m');
	lcd.print(mode);
	delay(1000);
}


void loop()
{


	displayMenu();

	if (millis() > timeOut) {
		timeOut = millis() + interval;


		switch (mode) {
			case 0:
				confetti();
				break;
			case 1:
				fill_solid(leds, NUM_LEDS, CHSV(value, 255, 255));
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
			mode = dbclicked % NUM_MODES;
			currentOption = 0;
			lcd.setCursor(14, 0);
			lcd.print('M');
			lcd.print(mode);
			break;
		case ClickEncoder::Held:
			break;
		}
	}

}

void confetti()
{
	// random colored speckles that blink in and fade smoothly
	fadeToBlackBy(leds, NUM_LEDS, 35);//long strip used five
	int pos = random16(NUM_LEDS);
	leds[pos] += CHSV(value + random8(64), 255, 255);
}

void displayMenu()
{
	int increment;
	value = encoder->getValue();
	if (value != 0) {
		last = value;
		lcd.setCursor(6, 0);
		lcd.print(value);
		lcd.print("    ");
	}

}