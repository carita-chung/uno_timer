#include <SevenSeg.h>
/*
  +_____________________________________________ 
  |   13 12 11 10  9  8| 7  6  5  4  3  2  1    +
  |                                             |
  |                         |              |    \
  |                         | .....        |     |
  |                         |              |     |
  |                      +-(0)-A--F-(1)(2)-B--+  |
  |                      |   _   _   _   _    |  |
  |                      |  |_| |_| |_| |_|   |  |
  |                      |  |_| |_| |_| |_|   |  |
  |                      |                    |  |
  |                      +--E--D-(.)-C--G-(3)-+  |
  |                         |              |     |
  |                         |              |     |
  |                         | .....        |     |
  |                                              |
  +_______________________|14 15 16 17 18 19____/
*/

SevenSeg disp(5, 1, 17, 15, 14, 4, 18);
int digitPins[] = { 6, 3, 2, 19, };
const int buttonPin = 7;
const int buzzerPin = 8;

#define LONG_PRESS	50
#define ADVANCE_MIN	1
int _tripped = ADVANCE_MIN * 60;

void setup() {
	disp.setDPPin(16);
	disp.setDigitPins(4, digitPins);
	pinMode(buttonPin, INPUT_PULLUP);
	pinMode(buzzerPin, OUTPUT);
	//disp.setCommonCathode();
	disp.setDutyCycle(20);
}

unsigned long saveMs = 0;
unsigned long pressed = 0;
#define M_SETUP		(1<<0)
#define M_DEC			(1<<1)
#define M_BUZZ		(1<<2)
#define M_LPRESS  (1<<3)
unsigned  _mode = M_SETUP;			// bit0 setup/run, bit1 decrement/stop, bit2 buzz/silent


void loop() {
	unsigned long nowMs = millis();
	if (nowMs - saveMs > 1000) {
		saveMs = nowMs;
		if (!(_mode&M_SETUP) && _mode&M_DEC) {
			_tripped--;
			if (!_tripped) {		// timer expired, stop decrement, enable buzzer
				_mode ^= M_DEC;
				_mode |= M_BUZZ;
			}//if
		}//if
	}//if
	if (pressed) {
		if (digitalRead(buttonPin) == HIGH) { 		// released
			if (pressed > 5) {		// to make sure it's not a glitch
				if (_mode&M_LPRESS) {
					if (_mode&M_SETUP) {
						_mode |= M_DEC;			// start count down
						_mode &= ~M_SETUP;	// not setup, now run mode
					}//if
					else {
						_mode = M_SETUP;		// back to setup mode
						_tripped = ADVANCE_MIN * 60;
					}//else
				}//if
				else {
					if (_mode&M_SETUP) {
						_tripped += ADVANCE_MIN * 60;		// add 5 minutes in setup
					}
					else {
						if (_tripped) _mode ^= M_DEC;		// start / stop timer
					}//else
				}//else
			}//if
			pressed = 0;
			_mode &= ~M_LPRESS;
		}//if
		else {
			pressed++;
			if (_mode&M_BUZZ) {
				noTone(buzzerPin);
				_mode = M_LPRESS;	// buzzer on, make it enter another round of setup
			}//if
			if (pressed >= LONG_PRESS) _mode |= M_LPRESS;
		}//else
	}//if
	else {
		if (digitalRead(buttonPin) == LOW) {
			pressed++;
		}//if
	}//else

	//___________________ shall we make some sound?
	if (_mode&M_BUZZ) {
		if (nowMs%500 >= 250) {
			tone(buzzerPin, 1000);
			disp.write("- - ");
		}//if
		else {
			noTone(buzzerPin);
			disp.write(" - -");
		}//else
		return;
	}//if

	//__________________ show remaining time
	if (_mode&M_SETUP && !(_mode&M_LPRESS) || (!(_mode&M_SETUP) && _mode&M_LPRESS)) {
		char buf[5];
		sprintf(buf, "%02d--", _tripped / 60);
			disp.write(buf);
	}//if
	else {
		disp.writeClock(_tripped);
	}//else

}

