CC = g++ -std=c++11

OBJ = main.o pwm.o sensor.o

#I= ../resource
#CFLAGS=  -Wall -lrt -pthread
CFLAGS = -Wall -pthread -lwiringPi

all : exo_exe	

.c.o :
	$(CC) $(CFLAGS)  -c -I$I -o $@ $< 

exo_exe : 	$(OBJ) 
	$(CC) $(CFLAGS) -s -o $@ $^ -lrt

#main.o:  main.c  
	#$(CC) -I$I -c -o $@ main.cpp
main.o:  main.cpp  
	$(CC) -c -o $@ main.cpp 
pwm.o:  PWM.cpp  
	$(CC) -c -o $@ PWM.cpp 
sensor.o :Sensor.cpp
	$(CC) -c -o $@ Sensor.cpp


clean :
	rm *.o  *.err
# eof
