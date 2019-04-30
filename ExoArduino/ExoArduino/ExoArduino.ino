/*
 Name:    	ExoArduino.ino
 Author:	Hao Lee
*/
#include <string.h>

//This is for the test pin
bool pinCond = false;
//

const int NUMSAMP = 3;
const int NUMSEN = 9;
const int SAMPINTERVAL = 5; //unit is ms
const unsigned long MAXTIME = 3600000; // Time reset every 1 hour, transmit less bits to increase sample freq
int sensorArray[] = {0,1,2,3,4,5,6,A15,A10};
int curIndex;

int senData[NUMSEN][NUMSAMP];
int senSum[NUMSEN];

int curCont = 1;
String sendResult;
String Z1 = String("0");
String Z2 = String("00");
String Z3 = String("000");
String Z4 = String("0000");
String Z5 = String("00000");
String Z6 = String("000000");
String Z7 = String("0000000");
String addZero(String curIn, unsigned int digit)
{
	String result=String("");
	switch ((digit-curIn.length()))
	{
	case 0:
		result= curIn;
		break;
	case 1:
		result = Z1 + curIn;
		break;
	case 2:
		result= Z2 + curIn;
		break;
	case 3:
		result = Z3 + curIn;
		break;
	case 4:
		result = Z4 + curIn;
		break;
	case 5:
		result= Z5 + curIn;
		break;
	case 6:
		result = Z6 + curIn;
		break;
	case 7:
		result = Z7 + curIn;
	default:
		break;
	}
	return result;
}
// the setup function runs once when you press reset or power the board
void setup() {

	Serial.begin(1000000,SERIAL_8E1);
	for (int i = 0; i < NUMSEN; i++) {
		for (int k = 0; k < NUMSAMP; k++)
			senData[i][k] = 0;
	}

	//I use this pin to test the frequency
	pinMode(50,OUTPUT);
	

	curIndex = 0;
	// TIMER 1 for interrupt frequency 200 Hz:
	cli(); // stop interrupts
	TCCR1A = 0; // set entire TCCR1A register to 0
	TCCR1B = 0; // same for TCCR1B
	TCNT1  = 0; // initialize counter value to 0
	// set compare match register for 200 Hz increments
	OCR1A = 9999; // = 16000000 / (8 * 200) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS12, CS11 and CS10 bits for 8 prescaler
	TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
	sei(); // allow interrupts

}

// the loop function runs over and over again until power down or reset
ISR(TIMER1_COMPA_vect){
	//sendResult = "@";
	// unsigned long curTime = millis()%MAXTIME;
	// sendResult += addZero(String(curTime),7);

	sendResult = "@7777777";
	for (int senIndex = 0; senIndex < NUMSEN; senIndex++) {
		
		senSum[senIndex] = senSum[senIndex] - senData[senIndex][curIndex];
		senData[senIndex][curIndex] = analogRead(sensorArray[senIndex]);
		senSum[senIndex] = senSum[senIndex] + senData[senIndex][curIndex];
		
		sendResult += addZero(String(senSum[senIndex] / NUMSAMP), 4);
		
	}
	sendResult = sendResult + "\n";
	Serial.print(sendResult);
	if(pinCond){
		digitalWrite(50,HIGH);
		pinCond = false;
	}
	else{
		digitalWrite(50,LOW);
		pinCond = true;
	}
	
	curIndex++; //This is index for moving average filter
	if (curIndex == NUMSAMP) 
		curIndex = 0;	

}
void loop()
{

}