#pragma once

/////////////////////////////////////////////////////////////////////
// MAX7219 8-digit 7-Segment LED display driver
// 2026.05.23
// https://github/mumanchu
// https://muman.ch

/*
Code translated from the C# of 2022.04.06
..\TinyCLRApplication3\MAX7219Led.cs

5V POWER
--------
The MAX7219 requires 5V, so the LED module must be powered by 5V ().
On a 3.3V MCU, use a 10K pullup to 3.3V on the chip select (CS) pin.
This allows more than one display to be connected, sharing the DIN 
and CLK signals, but with separate CS pins.

3-WIRE SERIAL INTERFACE
-----------------------
Clock (CLK), data in (DIN) and chip select (CS) lines.
The serial interface is not SPI, it is bit-banged (it does not need 
to be really fast). You can use 'CS' to select the chip if there is 
more than one chip on the same 2-wire serial bus (CLK & DIN). Use a 
10K pullup on the CS pin to 3.3V or 5V (depending on the MCU voltage).

DATA SHEET
----------
https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf
*/

class MAX7219Display
{
protected:
	uint dinPin;
	uint csPin;
	uint clkPin;

	static const byte charMap[];
	static const char* charSet;

public:
	bool begin(uint pinDIN, uint pinCS, uint pinCLK);
	void setDefaults();
	void setBrightness(uint brightness);
	void clearDisplay();
	void setShutdownMode(bool shutdown);
	void testDisplay(bool on);
	void setScanLimit(uint numberOfDigits);
	void setDecodeMode(uint decodeModeBits);
	void display7SegmentData(uint position, byte segData);
	void displayChar(uint position, char ch, bool dp = false);
	void displayText(const char* text);
	void displayText(const char* text, uint offset, uint length);

protected:
	void writeRegister(byte adds, byte data);
	void sendByte(byte b);
};

// NOTE: These bits are reversed with respect to the TM1637/8 chip!
// 
// 7-segment data for decimal, hex and other supported characters
// segments are numbered a..g, x is the decimal point '.'
// bit 76543210
//     xabcdefg
// 
//        a
//       ---
//    f |   | b
//       ---  g
//    e |   | c
//       ---  . x (dp)
//        d
// 
const byte MAX7219Display::charMap[] =
{
	//xabcdefg
	0b01111110,    // 0
	0b00110000,    // 1
	0b01101101,    // 2
	0b01111001,    // 3
	0b00110011,    // 4
	0b01011011,    // 5
	0b01011111,    // 6
	0b01110000,    // 7
	0b01111111,    // 8
	0b01111011,    // 9
	0b01110111,    // A
	0b00011111,    // b
	0b00001101,    // c
	0b00111101,    // d
	0b01001111,    // E
	0b01000111,    // F

	//xabcdefg
	0b00000000,    // space	[16]
	0b00000001,    // -
	0b00110111,    // H 
	0b00010111,    // h
	0b00110000,    // I
	0b00010000,    // i
	0b00111000,    // J
	0b00001110,    // L
	0b00110000,    // l
	0b01111110,    // O
	0b00011101,    // o
	0b01100111,    // P
	0b01110011,    // q
	0b00000101,    // r
	0b01011011,    // S
	0b00001111,    // t
	0b00111110,    // U
	0b00011100,    // u
	0b00111011,    // y
	0b01001110,    // C [35]
	0b01100011,    // \xb0 (degrees) [36]

	0b00000000     // NUL [37] 
};

// Supported characters
// offset into this string is the offset into charMap[]
const char* MAX7219Display::charSet =
//   0         1         2         3      
//   0123456789012345678901234567890123456
	"0123456789AbcdEF -HhIiJLlOoPqrStUuyC\xb0";
//   ----- hex ------


// Call from setup()
bool MAX7219Display::begin(uint pinDIN, uint pinCS, uint pinCLK)
{
	ASSERT(digitalPinToPinName(pinDIN) != NC && 
		digitalPinToPinName(pinCS) != NC &&
		digitalPinToPinName(pinCLK) != NC);
	dinPin = pinDIN;
	csPin = pinCS;
	clkPin = pinCLK;

	pinMode(dinPin, OUTPUT);
	digitalWrite(dinPin, 1);
	// chip select pin needs 10K pull-up!
	pinMode(csPin, OUTPUT_OPEN_DRAIN);
	digitalWrite(csPin, 1);
	pinMode(clkPin, OUTPUT);
	digitalWrite(clkPin, 0);

	setDefaults();
	return true;
}

// Set to a known default state
void MAX7219Display::setDefaults()
{
	setShutdownMode(false);
	setBrightness(0);
	setDecodeMode(0);
	clearDisplay();
	setScanLimit(8);
}

// brightness = 0..15 (0x00..0x0f)
void MAX7219Display::setBrightness(uint brightness)
{
	writeRegister(0x0a, brightness > 15 ? 15 : brightness);
}

// Note: For digits with BCD decode mode set, this sets the digit to '0'
void MAX7219Display::clearDisplay()
{
	for (uint i = 0; i < 8; ++i)
		writeRegister(1 + i, 0);
}

// Shutdown or normal operation
void MAX7219Display::setShutdownMode(bool shutdown)
{
	writeRegister(0x0c, shutdown ? 0 : 1);
}

// Turns all segments on or off
void MAX7219Display::testDisplay(bool on)
{
	writeRegister(0x0f, on ? 1 : 0);
}

// Defines how many digits will be used, from right to left
// >>> normally leave this at 8 digits
void MAX7219Display::setScanLimit(uint numberOfDigits)
{
	ASSERT2(numberOfDigits >= 1 && numberOfDigits <= 8);
	writeRegister(0x0b, numberOfDigits - 1);
}

// BCD decode mode, when set, uses only the lower 4 bits of the 
// digit register as a BCD value, plus bit 7 for the decimal point. 
// For blank, use 0x0f.
// 0x00..0x09 = '0'..'9'
// 0x0a = '-'
// 0x0b = 'E'
// 0x0c = 'H'
// 0x0d = 'L'
// 0x0e = 'P'
// 0x0f = ' ' (blank)
// decodeModeBits = one bit for each digit 76543210, from left to right 
void MAX7219Display::setDecodeMode(uint decodeModeBits)
{
	ASSERT2((uint)decodeModeBits <= 0xff);
	writeRegister(0x09, decodeModeBits);
}

// Digit position: 76543210
void MAX7219Display::display7SegmentData(uint position, byte segData)
{
	ASSERT2(position < 8);
	writeRegister(position + 1, segData);
}

// position = 7..0, 7=leftmost character, 0=rightmost character
void MAX7219Display::displayChar(uint position, char ch, bool dp /*= false*/)
{
	ASSERT2(position < 8);
	char* p = strchr(charSet, ch);
	byte segData = p ? charMap[p - charSet] : 0;
	writeRegister(position + 1, segData);
}

void MAX7219Display::displayText(const char* text)
{
	displayText(text, 0, 8);
}

void MAX7219Display::displayText(const char* text, uint offset, uint length)
{
	if (text == NULL) {
		clearDisplay();
		return;
	}
	if (length > 8)
		length = 8;
	uint pos = 0;		// 0 = rightmost character
	bool dp = false;

	for (uint i = offset + length - 1; i >= 0; i--) {
		char ch = text[i];
		if (ch == '.') {
			dp = true;
			continue;
		}
		displayChar(pos, ch, dp);
		if (++pos == 8)
			break;
		dp = false;
	}
	while (pos < 8)
		displayChar(pos++, ' ', false);
}

// Internal methods

void MAX7219Display::writeRegister(byte adds, byte data)
{
	ASSERT2(adds <= 0x0f);
	digitalWrite(csPin, 0);
	sendByte(adds);
	sendByte(data);
	digitalWrite(csPin, 1);
}

void MAX7219Display::sendByte(byte b)
{
	// clock out each bit, MS bit first
	for (uint bit = 0; bit < 8; ++bit) {
		digitalWrite(clkPin, 0);
		digitalWrite(dinPin, (b & 0x80) ? 1 : 0);
		digitalWrite(clkPin, 1);
		b <<= 1;
	}
}

