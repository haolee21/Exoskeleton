#ifndef COMMON_H
#define COMMON_H
/*This header define the common function that shares among all objects*/
#include <time.h>

//this function is build for linux, ignore the error in windows


/*
void DelaySys(int waitTime) {
	struct timespec ts2 = { 0 };
	ts2.tv_sec = waitTime;
	ts2.tv_nsec = 10000L; //10 us
	nanosleep(&ts2, (struct timespec*)NULL);
}*/
//Function for serial port communication in linux
/*====================================================================================================*/
		/* Serial Port Programming in C (Serial Port Write)                                                   */
	/* Non Cannonical mode                                                                                */
	/*----------------------------------------------------------------------------------------------------*/
		/* Program writes a character to the serial port at 9600 bps 8N1 format                               */
	/* Baudrate - 9600                                                                                    */
	/* Stop bits -1                                                                                       */
	/* No Parity                                                                                          */
		/*----------------------------------------------------------------------------------------------------*/
	/* Compiler/IDE  : gcc 4.6.3                                                                          */
	/* Library       :                                                                                    */
	/* Commands      : gcc -o serialport_write serialport_write.c                                         */
	/* OS            : Linux(x86) (Linux Mint 13 Maya)(Linux Kernel 3.x.x)                                */
	/* Programmer    : Rahul.S                                                                            */
	/* Date	         : 21-December-2014                                                                   */
	/*====================================================================================================*/

	/*====================================================================================================*/
	/* Running the executable                                                                             */
	/* ---------------------------------------------------------------------------------------------------*/
	/* 1) Compile the  serialport_read.c  file using gcc on the terminal (without quotes)                 */
		/*                                                                                                    */
	/*	" gcc -o serialport_write serialport_write.c "                                                */
	/*                                                                                                    */
		/* 2) Linux will not allow you to access the serial port from user space,you have to be root.So use   */
		/*    "sudo" command to execute the compiled binary as super user.                                    */
		/*                                                                                                    */
		/*       "sudo ./serialport_write"                                                                    */
	/*                                                                                                    */
	/*====================================================================================================*/

	/*====================================================================================================*/
	/* Sellecting the Serial port Number on Linux                                                         */
	/* ---------------------------------------------------------------------------------------------------*/
	/* /dev/ttyUSBx - when using USB to Serial Converter, where x can be 0,1,2...etc                      */
	/* /dev/ttySx   - for PC hardware based Serial ports, where x can be 0,1,2...etc                      */
		/*====================================================================================================*/

	/*-------------------------------------------------------------*/
		/* termios structure -  /usr/include/asm-generic/termbits.h    */
	/* use "man termios" to get more info about  termios structure */
	/*-------------------------------------------------------------*/







#endif // !COMMON_H
