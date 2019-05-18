/*
 Name:    	ExoArduino.ino
 Author:	Hao Lee
*/
#include <string.h>
char buffer[100];
char *loc;
unsigned long currentTime;
int val = 0;
// the setup function runs once when you press reset or power the board
void setup() {

	Serial.begin(115200,SERIAL_8E1);


	//I use this pin to test the frequency
	pinMode(50,OUTPUT);
	


	// TIMER 1 for interrupt frequency 10 Hz:
    cli(); // stop interrupts
    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0; // same for TCCR1B
    TCNT1  = 0; // initialize counter value to 0
    // set compare match register for 10 Hz increments
    OCR1A = 24999; // = 16000000 / (64 * 10) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS12, CS11 and CS10 bits for 64 prescaler
    TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
    sei(); // allow interrupts

}

// the loop function runs over and over again until power down or reset
ISR(TIMER1_COMPA_vect){
    currentTime = millis();
    val = analogRead(0);
    sprintf(buffer,"%07lu,%04d",currentTime,val>>2);
    
    Serial.println(buffer);
		

}
void loop()
{

}