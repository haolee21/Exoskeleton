/*
 Name:    	ExoArduino.ino
 Author:	Hao Lee


 // important: when using platformio, use: platformio init --board megaADK      to create project

*/
#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif




//This is for the test pin
bool pinCond = true;
//


const int NUMSEN = 16;

int sensorArray[] = {0, 1, 2, 3, 4, 5, 6,7,8,9,10,11,12,13,14,15};
int curIndex;


char buffer[100];
char *bufferPointer;
int curCont = 1;

bool readyToSend;

// buffer and array for sensing
union SenDataType {
	int senVal;
	char senByte[4];
};
// union TimeDataType {
// 	unsigned long timeVal;
// 	char timeByte[4];
// };
// TimeDataType curTime;
SenDataType curSen;
int testSent1;
int testSent2;
// the setup function runs once when you press reset or power the board
void setup()
{
	#if FASTADC
 	// set prescale to 16
 	sbi(ADCSRA,ADPS2) ;
 	cbi(ADCSRA,ADPS1) ;
 	cbi(ADCSRA,ADPS0) ;
	#endif

	testSent1 = 97;
	testSent2 = 65;
	curIndex = 0;
	bufferPointer = buffer;
	Serial.begin(576000, SERIAL_8E1);
	// for (int i = 0; i < NUMSEN; i++)
	// {
	// 	for (int k = 0; k < NUMSAMP; k++)
	// 		senData[i][k] = 0;
	// }
	readyToSend = false;
	//I use this pin to test the frequency
	// pinMode(50, OUTPUT);
	// pinMode(51,OUTPUT);
	// Timer setting: http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html

// TIMER 1 for interrupt frequency 800 Hz:
cli(); // stop interrupts
TCCR1A = 0; // set entire TCCR1A register to 0
TCCR1B = 0; // same for TCCR1B
TCNT1  = 0; // initialize counter value to 0
// set compare match register for 800 Hz increments
OCR1A = 19999; // = 16000000 / (1 * 800) - 1 (must be <65536)
// turn on CTC mode
TCCR1B |= (1 << WGM12);
// Set CS12, CS11 and CS10 bits for 1 prescaler
TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
// enable timer compare interrupt
TIMSK1 |= (1 << OCIE1A);
sei(); // allow interrupts
}

// the loop function runs over and over again until power down or reset
ISR(TIMER1_COMPA_vect)
{
	readyToSend = true;
}
void loop()
{

	if (readyToSend)
	{
		// digitalWrite(51,HIGH);
		// // testSent is for testing the receiving end got correct data
		// testSent1++;
		// testSent2++;
		// if (testSent1 > 122)
		// 	testSent1 = 97;
		// if (testSent2 > 90)
		// 	testSent2 = 65;

		

		for (int senIndex = 0; senIndex < NUMSEN; senIndex++)
		{
			curSen.senVal = analogRead(sensorArray[senIndex]);
			*bufferPointer++ = curSen.senByte[0];
			*bufferPointer++ = curSen.senByte[1];

		}
		*bufferPointer++ = '\n';
		//Create the output data
		// digitalWrite(51,LOW);
		// if (pinCond)
		// {
		// 	digitalWrite(50, HIGH);
		// 	pinCond = false;
		// }
		// else
		// {
		// 	digitalWrite(50, LOW);
		// 	pinCond = true;
		// }
		
		Serial.write(buffer,NUMSEN*2+1);
		readyToSend = false;
		bufferPointer = buffer;
	}
}