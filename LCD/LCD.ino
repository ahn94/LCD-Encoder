#define WITH_LCD 1
// See Library "Docs" folder for possible commands etc.
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>


#define LCD_CHARS   16
#define LCD_LINES    2


// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

ClickEncoder *encoder;
uint16_t last, value;

void timerIsr() {
  encoder->service();
}



void setup()
{
	Serial.begin(115200);
	// initialize library
	lcd.begin(LCD_CHARS, LCD_LINES);
	encoder = new ClickEncoder(A1, A0, A2, 4);
	encoder->setAccelerationEnabled(true);
	Timer1.initialize(1000);
	Timer1.attachInterrupt(timerIsr);

	last = -1;

	lcd.backlight();
	// set cursor to positon x=0, y=0
	lcd.setCursor(0, 0);
	// print Hello!
	lcd.print("value=");
	// wait a second.
	delay(1000);
}


void loop()
{
	value += encoder->getValue();

	if (value != last) {
		last = value;
		lcd.setCursor(6, 0);
		lcd.print(value);
		lcd.print("    ");
		Serial.print("Encoder Value: ");
		Serial.println(value);
		Serial.println("--------------");
	}
}