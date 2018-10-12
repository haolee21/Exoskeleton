/*
 Name:		ExoArduino.ino
 Author:	Hao Lee
*/
#include <string.h>
const int NUMSAMP = 3;
const int NUMSEN = 9;
const int SAMPINTERVAL = 8; //unit is ms
//const int SAMPINTERVAL = 100; //unit is ms
const unsigned long MAXTIME = 3600000; // Time reset every 1 hour, transmit less bits to increase sample freq
int sensorArray[] = {0,1,2,3,4,5,6,A15,A10};
int curIndex;

int senData[NUMSEN][NUMSAMP];
int senSum[NUMSEN];
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
	Serial.begin(115200,SERIAL_8E1);
	for (int i = 0; i < NUMSEN; i++) {
		for (int k = 0; k < NUMSAMP; k++)
			senData[i][k] = 0;
	}
	curIndex = 0;
	
}

// the loop function runs over and over again until power down or reset
void loop() {
	sendResult = "@";
	unsigned long curTime = millis()%MAXTIME;
	sendResult += addZero(String(curTime),7);
	
	for (int senIndex = 0; senIndex < NUMSEN; senIndex++) {
		
		senSum[senIndex] = senSum[senIndex] - senData[senIndex][curIndex];
		senData[senIndex][curIndex] = analogRead(sensorArray[senIndex]);
		senSum[senIndex] = senSum[senIndex] + senData[senIndex][curIndex];
		
		sendResult += addZero(String(senSum[senIndex] / NUMSAMP), 4);
		//sendResult += addZero(String((double)senSum[senIndex] / NUMSAMP, 0),3);
	}
	
	
	sendResult = sendResult + "\n";
	//sendResult = "@11111,222,333,444,555,666,777,888,999,123\n";
	//sendResult = Z1+ sendResult;
	Serial.print(sendResult);
	curIndex++;
	if (curIndex == NUMSAMP) 
		curIndex = 0;	
	
	
	unsigned int waitTime = (millis() % MAXTIME) -curTime;
	while ((waitTime < SAMPINTERVAL)&&waitTime>0) {
		delayMicroseconds(5);
		waitTime = millis() - curTime;
	}
}
