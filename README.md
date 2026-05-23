# MAX7219Display
MAX7219 8-digit 7-Segment LED display driver.

![max7219-led](/images/max7219-led.jpg)


## 5V Power

The MAX7219 requires 5V, so the LED module must be powered by 5V.
On a 3.3V MCU, use a 10K pullup to 3.3V on the chip select (CS) pin.
On a 5V MCU, connect the 10K pullup to 5V.
The CS ouput is 'open dran'. This allows more than one display to be 
connected, sharing the DIN and CLK signals, but with separate CS pins.

## 3-Wire Serial Interface

It has Clock (CLK), data in (DIN) and chip select (CS) signals.
The serial interface is not SPI, it is bit-banged (it does not need 
to be really fast). You can use 'CS' to select the chip if there is 
more than one chip on the same 2-wire serial bus (CLK & DIN). Use a 
10K pullup on the CS pin to 3.3V or 5V (depending on the MCU voltage).

## Class Reference

```cpp
class MAX7219Display
{
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
};
```
`displayText()` can contain the decimal point.

## Data Sheet

https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf

## Revision History

| Date       | Version  | Details |
|:---------- |:---------|:----------- |
| 2026.05.23 | 0.0.0	| Preliminary |

<br/>


## Joke of the Week

_My brain has a mind of its own._
