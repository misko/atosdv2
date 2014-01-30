#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

#include <math.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define CHANNEL 0

#define M1PIN	27
#define M2PIN	2
#define ENPIN	22


uint8_t buf[2];
char spos[] = "7=0.063\n";
char epos[] = "7=0.21\n";
char stop[] = "7=0\n";

long start_time=0;

float stddev;
float mean; 
#define CALIBRATION_SIZE 20
float sensor_calibration[CALIBRATION_SIZE];


#define RES	25

long gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	//return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	return tv.tv_sec;
}


float update_stats(float * x, int n, float * out_mean, float * out_stddev) {
    int  i;
    float average, variance, std_deviation, sum = 0, sum1 = 0;
    for (i = 0; i < n; i++) {
        sum = sum + x[i];
    }
    average = sum / (float)n;
    /*  Compute  variance  and standard deviation  */
    for (i = 0; i < n; i++) {
        sum1 = sum1 + pow((x[i] - average), 2);
    }
    variance = sum1 / (float)n;
    std_deviation = MAX(sqrt(variance),3);
    *out_mean=average;
    *out_stddev=std_deviation;
 
    return std_deviation;
}

unsigned int sample(int channel) {
	uint8_t spiData [2] ;

	uint8_t chanBits ;

	if (channel == 0)
		chanBits = 0b11010000 ;
	else
		chanBits = 0b11110000 ;

	spiData [0] = chanBits ;
	spiData [1] = 0 ;

	wiringPiSPIDataRW (0, spiData, 2) ;

	return ((spiData [0] << 7) | (spiData [1] >> 1)) & 0x3FF ;
}

void cookieInterrupt(void) { 
	//Got a cookie!
	printf("got a cookie!\n");
	//setpos(stop);
	exit(1);
}

void calibrate_sensor(int aq) {
	//calibrate the sensor
	if (aq==1) {
		int i=0;
		for (i=0; i<CALIBRATION_SIZE; i++) {
			sensor_calibration[i] = sample(0);
			delay(RES);
		}
	}
	update_stats(sensor_calibration,CALIBRATION_SIZE,&mean,&stddev);
	fprintf(stdout, "u/std %f/%f\n", mean, stddev);
}

int state=0;

void move() {
	fprintf(stdout,"move()\n");
	//change state and start spining
	digitalWrite(ENPIN,0);	
	digitalWrite(M1PIN,state);
	digitalWrite(M2PIN,1-state);
	digitalWrite(ENPIN,1);	


	int i;
	//unsigned int prev=0x4FF;
	int init_back_down=10;
	int back_down=init_back_down;
	for (i=0; i*RES<1000; i++) {
		delay(RES);
		unsigned int current = sample(1);
		unsigned int sensor = sample(0);
		printf("sample=%d vs sensor=%d\n", current, sensor); //get the read from the motor
		if (abs(sensor-mean)>6*stddev) {
			delay(1);
			unsigned int sensor2 = sample(0);
			if (abs(sensor2-mean)>6*stddev) {
				printf("COOKIE DROP!\n");
				digitalWrite(ENPIN,0);	
				exit(1);
			}
			
		}	
		//unsigned int mx=MAX(current,prev);
		//unsigned int mn=MIN(current,prev);
		//if (mx-mn<50) {
		if (current<730) {
			back_down--;
			if (back_down==0) {
				printf("jam\n");
				state=1-state;
				return;
			}
		} else {
			back_down=init_back_down;
		}
		//prev=current;
	}
}



int main (int argc, char ** argv) {

	if (argc!=2) {
		printf("%s timeout\n",argv[0]);
		exit(1);
	}

	long timeout = atol(argv[1]);

	if (timeout==0 || timeout>10) {
		printf("timeout must be in range [1,10]\n");
		exit(1);
	}	

	wiringPiSetupGpio();
	//lets setup enable pin
	pinMode(M1PIN,OUTPUT);
	pinMode(M2PIN,OUTPUT);
	pinMode(ENPIN,OUTPUT);
	digitalWrite(ENPIN,0); //Turn the motor off
	
	if (wiringPiSPISetup(CHANNEL, 1000000) < 0) {
		fprintf (stderr, "SPI Setup failed: %s\n", strerror (errno));
		exit(errno);
	}

	start_time=gettime();


	calibrate_sensor(1);
	
	while ( (gettime()-start_time)<=timeout) {
		move();
	}

	digitalWrite(ENPIN,0); //Turn the motor off
	return 0;
}



