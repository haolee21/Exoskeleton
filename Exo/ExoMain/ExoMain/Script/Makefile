CC = g++ -std=c++11

OBJ = main.o pwm.o sensor.o controller.o valve.o

I= /home/pi/Lib/boost_1_70_0
#CFLAGS=  -Wall -lrt -pthread
CFLAGS = -Wall -pthread -lwiringPi -g

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
controller.o: Controller.cpp
	$(CC) -c -o $@ Controller.cpp
valve.o: Valve.cpp
	$(CC) -c -o $@ Valve.cpp

clean :
	rm *.o  *.err
# eof
