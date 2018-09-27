/*
 Name:		ExoArduino.ino
 Author:	Hao Lee
*/
const int NUMSAMP = 3;
const int NUMSEN = 9;
const int SAMPINTERVAL = 5; //unit is ms
const unsigned long MAXTIME = 3600000; // Time reset every 1 hour, transmit less bits to increase sample freq
int sensorArray[] = {A0,A1,A2,A3,A4,A5,A6,A7,A8};
int curIndex;

int senData[NUMSEN][NUMSAMP];
int senSum[NUMSEN];
String sendResult;
// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
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
	sendResult += String(curTime);
	
	for (int senIndex = 0; senIndex < NUMSEN; senIndex++) {
		sendResult += ",";
		senSum[senIndex] = senSum[senIndex] - senData[senIndex][curIndex];
		senData[senIndex][curIndex] = analogRead(sensorArray[senIndex]);
		senSum[senIndex] = senSum[senIndex] + senData[senIndex][curIndex];
		sendResult += String((double)senSum[senIndex] / NUMSAMP, 0);
	}
	sendResult = sendResult + "\n";
	//sendResult = "@0,1,2,3,4,5,6,7,8,9\n";
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
