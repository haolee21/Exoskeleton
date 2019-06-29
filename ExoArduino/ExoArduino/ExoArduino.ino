/*
 Name:    	ExoArduino.ino
 Author:	Hao Lee
*/

//This is for the test pin
bool pinCond = true;
//

const int NUMSAMP = 4;
const int NUMSEN = 9;
const int SAMPDIV = 2; //This actually take 4 samples, use measurement>>2 to divide by 4
int sensorArray[] = {0, 1, 2, 3, 4, 5, 6, A15, A10};
int curIndex;

int senData[NUMSEN][NUMSAMP];
int senSum[NUMSEN];
char buffer[100];
char *bufferPointer;
int curCont = 1;

bool readyToSend;

// buffer and array for sensing
union SenDataType {
	int senVal;
	char senByte[4];
};
union TimeDataType {
	unsigned long timeVal;
	char timeByte[4];
};
TimeDataType curTime;
SenDataType curSen;
int testSent1;
int testSent2;
// the setup function runs once when you press reset or power the board
void setup()
{

	testSent1 = 97;
	testSent2 = 65;
	curIndex = 0;
	bufferPointer = buffer;
	Serial.begin(1000000, SERIAL_8E1);
	for (int i = 0; i < NUMSEN; i++)
	{
		for (int k = 0; k < NUMSAMP; k++)
			senData[i][k] = 0;
	}
	readyToSend = false;
	//I use this pin to test the frequency
	pinMode(50, OUTPUT);

	// Timer setting: http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
// TIMER 1 for interrupt frequency 625 Hz:
cli(); // stop interrupts
TCCR1A = 0; // set entire TCCR1A register to 0
TCCR1B = 0; // same for TCCR1B
TCNT1  = 0; // initialize counter value to 0
// set compare match register for 625 Hz increments
OCR1A = 25599; // = 16000000 / (1 * 625) - 1 (must be <65536)
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
		// testSent is for testing the receiving end got correct data
		testSent1++;
		testSent2++;
		if (testSent1 > 122)
			testSent1 = 97;
		if (testSent2 > 90)
			testSent2 = 65;

		*bufferPointer++ = '@';
		curTime.timeVal = micros();

		for (int sendIndex = 0; sendIndex < 4; sendIndex++)
		{
			*bufferPointer++ = curTime.timeByte[sendIndex];
			//*bufferPointer++ = testSent1;
		}
		for (int senIndex = 0; senIndex < NUMSEN; senIndex++)
		{
			senSum[senIndex] = senSum[senIndex] - senData[senIndex][curIndex];
			senData[senIndex][curIndex] = analogRead(sensorArray[senIndex]);

			senSum[senIndex] = senSum[senIndex] + senData[senIndex][curIndex];
			curSen.senVal = senSum[senIndex] >> SAMPDIV;
			for (int sendIndex = 0; sendIndex < 2; sendIndex++)
			{
				*bufferPointer++ = curSen.senByte[sendIndex];
				//*bufferPointer++ = testSent2;
			}
		}
		*bufferPointer++ = '\n';
		//Create the output data

		if (pinCond)
		{
			digitalWrite(50, HIGH);
			pinCond = false;
		}
		else
		{
			digitalWrite(50, LOW);
			pinCond = true;
		}
		curIndex++; //This is index for moving average filter
		if (curIndex == NUMSAMP)
			curIndex = 0;
		Serial.write(buffer, 24);
		readyToSend = false;
		bufferPointer = buffer;
	}
}