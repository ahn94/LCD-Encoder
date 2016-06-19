#include <FastLED.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>

#define LCD_CHARS   20		// columns
#define LCD_LINES    4		// rows
#define NUM_LEDS 147		// number of leds
#define DATA_PIN 2			// led data pin
#define LED_TYPE    WS2812B	// led type

const int toggleRight = 12;
const int toggleLeft = 10;

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

ClickEncoder *encoder;	// pointer to encoder
uint8_t increment;		// encoder output stored here every loop

// general menu variables
#define NUM_MODES 5  				// number of modes;
uint8_t nOptions[] = { 4, 3, 3 , 1, 2};	// options per mode;
uint8_t currentOption;				// option for current mode
uint8_t mode = 4;					// current mode
uint8_t clicked = 0;				// used to % = cycle through options for current mode
uint8_t dbclicked = 0;				// used to % = cycle through modes
long timeOut = -1;					// timout
uint8_t setting = mode;
bool isLOW;
uint8_t fading = 0;



const uint8_t MODE = 0;
const uint8_t HUE = 1;
const uint8_t SAT = 2;
const uint8_t BRIGHT = 3;
const uint8_t DELAY = 4;
const uint8_t DELTA = 5;

// settings 
// 0-mode, 1-hue, 2-saturation, 3-brightness, 4-interval, 5-delataHue
uint8_t settings[][6] = 
{
	{ 0, 87, 255, 255, 26, 0},	// confetti
	{ 1, 0, 255, 255, 255, 0},	// solid
    { 2, 0, 255, 180, 40, 2},	// rainbow
    { 3, 0, 255, 255, 49, 0},	// pulse
	{ 0, 240, 255, 255, 26, 0},	// confetti - red
	{ 0, 170, 255, 255, 26, 0}	// confetti blue
};

void timerIsr() {
  encoder->service();
}

void setup()
{
	// initialize LEDS
	LEDS.setBrightness(255);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

	pinMode(toggleRight, INPUT);
	pinMode(toggleLeft, INPUT);
	if (digitalRead(toggleLeft) == LOW) {
		isLOW = true;
	} else {
		isLOW = false;
	}
	Serial.begin(115000);

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
}


void loop() 
{
	Serial.println("---");
	int rt = digitalRead(toggleRight);
	int lt = digitalRead(toggleLeft);

	if (rt == LOW) {
		lcd.off();
	} else {
		lcd.on();
	}

	if (lt == LOW) {
		if (isLOW == false) {
			isLOW = true;
			fading = settings[setting][BRIGHT];
		}
		if (settings[setting][BRIGHT] - fading > 0) {
			fading += 1;
		}
	}

	if (lt == HIGH) {
		if (fading > 0) {
			fading -= 1;
		}
	}

	increment = encoder->getValue();
	if (increment != 0) {
		display(increment);
	}

	if (millis() > timeOut) {
		// sets timout from selected option
		// passes array with mode settings to animation method
		timeOut = millis() + settings[setting][DELAY];
		animate(settings[setting]);
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
			setting = mode;
			display(0);
			break;
		case ClickEncoder::Held:
			break;
		}
	}

}

void animate(uint8_t current[]) 
{
	Serial.println(current[BRIGHT]);
	switch (current[0]) {
	case 0:
		fadeToBlackBy(leds, NUM_LEDS, 2);//long strip used fives
		leds[random16(NUM_LEDS)] +=
			CHSV(current[HUE] + random8(64), current[SAT], current[BRIGHT]- fading);
		break;
	case 1:
		fill_solid(leds, NUM_LEDS, CHSV(current[HUE], current[SAT], current[BRIGHT]- fading));
		break;
	case 2:
		fill_rainbow(leds, NUM_LEDS, current[HUE], current[DELTA]);
		FastLED.setBrightness(current[BRIGHT] - fading);
		current[HUE] += 1;
		break;
	case 3:
		fill_solid(leds, NUM_LEDS, CHSV(current[HUE], current[SAT], current[BRIGHT] - fading));
		current[HUE] += 1;
		break;
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
					settings[mode][HUE] += incr;
					adjustHue(settings[mode][HUE]); // print color display
					break;
				case 1: // saturation option
					settings[mode][SAT] += incr;
					adjustSaturation(settings[mode][SAT]); // print saturation display
					break;
				case 2: // brightness option
					settings[mode][BRIGHT] += incr;
					adjustBrightness(settings[mode][BRIGHT]); // print brightness display 
					break;
				case 3: // interval option
					settings[mode][DELAY] += incr;
					adjustInterval(settings[mode][DELAY]); // print interval option
					break;
			}
			lcd.setCursor(0, 0);
			lcd.print("      CONFETTI      ");
			break;
		case 1: // solid color mode
			switch (currentOption) // Options displayer switch
			{
				case 0: // color option
					settings[mode][HUE] += incr;
					adjustHue(settings[mode][HUE]); // print color display
					break;
				case 1: // saturation option
					settings[mode][SAT] += incr;
					adjustSaturation(settings[mode][SAT]); // print saturation display
					break;
				case 2: // brightness option
					settings[mode][BRIGHT] += incr;
					adjustBrightness(settings[mode][BRIGHT]); // print brightness display 
					break;
			}
			lcd.setCursor(0, 0);
			lcd.print("       SOLID       ");
			break;
		case 2: // rainbow mode
			switch (currentOption)
			{
				case 0: // interval option
					settings[mode][DELAY] += incr;
					adjustInterval(settings[mode][DELAY]); // print interval option
					break;
				case 1: // huedelta option
					settings[mode][DELTA] += incr;
					adjustDeltaHue(settings[mode][DELTA]); // print saturation display
					break;
				case 2: // brightness option
					settings[mode][BRIGHT] += incr;
					adjustBrightness(settings[mode][BRIGHT]); // print brightness display 
					break;
			
			}
			lcd.setCursor(0, 0);
			lcd.print("      RAINBOW      ");
			break;
		case 3: // rainbow pulse
			settings[mode][DELAY] += incr;
			adjustInterval(settings[mode][DELAY]);
			lcd.setCursor(0, 0);
			lcd.print("       PULSE        ");
			break;
		case 4:
			setting = currentOption + 4;
			lcd.setCursor(0, 0);
			lcd.print("      PRESETS        ");
			switch (setting) 
			{
				case 4:
					lcd.setCursor(9, 2);
					lcd.print("CONFETTI   ");
					lcd.setCursor(9, 3);
					lcd.print("[red]           ");
					break;
				case 5:
					lcd.setCursor(9, 2);
					lcd.print("CONFETTI   ");
					lcd.setCursor(9, 3);
					lcd.print("[blue]           ");
					break;
			}


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

}

void adjustHue(uint8_t hue)
{
	lcd.setCursor(9, 2);
	lcd.print("COLOR     ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(hue);
	lcd.print("]    ");
}

void adjustSaturation(uint8_t sat)
{
	lcd.setCursor(9, 2);
	lcd.print("SATURATION   ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(sat);
	lcd.print("]    ");
}

void adjustBrightness(uint8_t bright)
{
	lcd.setCursor(9, 2);
	lcd.print("BRIGHTNESS     ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(bright);
	lcd.print("]    ");
}

void adjustInterval(uint8_t interval)
{
	lcd.setCursor(9, 2);
	lcd.print("SPEED       ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(interval);
	lcd.print("]    ");
}

void adjustDeltaHue(uint8_t deltaHue)
{
	lcd.setCursor(9, 2);
	lcd.print("DELTAHUE      ");
	lcd.setCursor(9, 3);
	lcd.print("[");
	lcd.print(deltaHue);
	lcd.print("]    ");
}