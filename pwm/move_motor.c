/*
 * softPwm.c:
 *	Test of the software PWM driver. Needs 8 LEDs connected
 *	to the Pi - e.g. Ladder board.
 *
 * Copyright (c) 2012-2013 Gordon Henderson. <projects@drogon.net>
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <wiringPi.h>
#include <softPwm.h>





int main (int argc, char ** argv) {

  if (argc!=3) {
	printf("%s motor pos\n",argv[0]);
	exit(1);
  }

  int motor=atoi(argv[1]);
  int pos=atoi(argv[2]);
 
  if (motor<3 || motor >6) {
	fprintf(stderr,"invalid motor, must be 3,4,5,6\n");
	exit(1);
  }

  if (pos<0 || pos>100) {
	fprintf(stderr, "invalid pos, must be [0,100]\n");
	exit(1);
  }

  if (wiringPiSetup()==-1)  {
	fprintf(stderr, "failed to setup wiring pi\n");
	exit(1);
  }


  softPwmCreate(motor,0,249);
  pwmSetMode(PWM_MODE_BAL);
  pwmSetClock(100);
  softPwmWrite(motor, pos);
  delay(1000);
  return 0; 
}
