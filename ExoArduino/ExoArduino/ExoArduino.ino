/*
 Name:    	ExoArduino.ino
 Author:	Hao Lee
*/


//This is for the test pin
bool pinCond = false;
//

const int NUMSAMP = 4; 
const int NUMSEN = 9;
const int SAMPDIV = 2;//This actually take 4 samples, use measurement>>2 to divide by 4
int sensorArray[] = {0,1,2,3,4,5,6,A15,A10};
int curIndex;
unsigned long curTime;
int senData[NUMSEN][NUMSAMP];
int senSum[NUMSEN];
char buffer[100];
int curCont = 1;

bool readyToSend;
// the setup function runs once when you press reset or power the board
void setup() {

	Serial.begin(1000000,SERIAL_8E1);
	for (int i = 0; i < NUMSEN; i++) {
		for (int k = 0; k < NUMSAMP; k++)
			senData[i][k] = 0;
	}
	readyToSend = false;
	//I use this pin to test the frequency
	pinMode(50,OUTPUT);
	

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
	
	curTime = millis()%MAXTIME;
	
	for (int senIndex = 0; senIndex < NUMSEN; senIndex++) {
		
		senSum[senIndex] = senSum[senIndex] - senData[senIndex][curIndex];
		senData[senIndex][curIndex] = analogRead(sensorArray[senIndex]);
		senSum[senIndex] = senSum[senIndex] + senData[senIndex][curIndex];
	}
	//Create the output data 
	sprintf(buffer,"@%07lu%04d%04d%04d%04d%04d%04d%04d%04d",curTime,senSum[0]>>SAMPDIV,senSum[1]>>SAMPDIV,senSum[2]>>SAMPDIV,senSum[3]>>SAMPDIV,senSum[4]>>SAMPDIV,senSum[5]>>SAMPDIV,senSum[6]>>SAMPDIV,senSum[7]>>SAMPDIV);
	readyToSend = true;
	
	
	// if(pinCond){
	// 	digitalWrite(50,HIGH);
	// 	pinCond = false;
	// }
	// else{
	// 	digitalWrite(50,LOW);
	// 	pinCond = true;
	// }
	
	curIndex++; //This is index for moving average filter
	if (curIndex == NUMSAMP) 
		curIndex = 0;	

}
void loop()
{
	if (readyToSend){
		Serial.println(buffer);
		readyToSend = false;
	}

}