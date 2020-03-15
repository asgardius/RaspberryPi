/*
=================================================================================
 Name        : pcd8544_rpi.c
 Version     : 0.1

 Copyright (C) 2012 by Andre Wussow, 2012, desk@binerry.de

 Description :
     A simple PCD8544 LCD (Nokia3310/5110) for Raspberry Pi for displaying some system informations.
	 Makes use of WiringPI-library of Gordon Henderson (https://projects.drogon.net/raspberry-pi/wiringpi/)

	 Recommended connection (http://www.raspberrypi.org/archives/384):
	 LCD pins      Raspberry Pi
	 LCD1 - GND    P06  - GND
	 LCD2 - VCC    P01 - 3.3V
	 LCD3 - CLK    P11 - GPIO0
	 LCD4 - Din    P12 - GPIO1
	 LCD5 - D/C    P13 - GPIO2
	 LCD6 - CS     P15 - GPIO3
	 LCD7 - RST    P16 - GPIO4
	 LCD8 - LED    P01 - 3.3V 

================================================================================
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
================================================================================
 */
#include <wiringPi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "PCD8544.h"
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

// pin setup
int _din = 1;
int _sclk = 0;
int _dc = 2;
int _rst = 4;
int _cs = 3;
  
// lcd contrast 
int contrast = 50;
  
int main (void)
{
  //clock
  // variables to store date and time components
  int hours, minutes, seconds, day, month, year;
  
  //network
  int fdl;
  int fdr;
  struct ifreq ifrl;
  struct ifreq ifrr;
  
  // print infos
  printf("Raspberry Pi PCD8544 sysinfo display\n");
  printf("========================================\n");
  
  // check wiringPi setup
  if (wiringPiSetup() == -1)
  {
	printf("wiringPi-Error\n");
    exit(1);
  }
  
  // init and clear lcd
  LCDInit(_sclk, _din, _dc, _cs, _rst, contrast);
  LCDclear();
  
  // show logo
  LCDshowLogo();
  
  delay(2000);
  
  for (;;)
  {
	  char wrinfo[15];
	  char wlinfo[15];
	  
	  fdl = socket(AF_INET, SOCK_DGRAM, 0);
	  
	  /* I want to get an IPv4 IP address */
	  ifrl.ifr_addr.sa_family = AF_INET;
	  
	  /* I want IP address attached to "wlan0" */
	  strncpy(ifrl.ifr_name, "wlan0", IFNAMSIZ-1);
	  
	  ioctl(fdl, SIOCGIFADDR, &ifrl);
	  close(fdl);
	  sprintf(wlinfo, "%s", inet_ntoa(((struct sockaddr_in *)&ifrl.ifr_addr)->sin_addr));
	  
	  fdr = socket(AF_INET, SOCK_DGRAM, 0);
	  
	  /* I want to get an IPv4 IP address */
	  ifrr.ifr_addr.sa_family = AF_INET;
	  
	  /* I want IP address attached to "wlan0" */
	  strncpy(ifrr.ifr_name, "eth0", IFNAMSIZ-1);
	  
	  ioctl(fdr, SIOCGIFADDR, &ifrr);
	  close(fdr);
	  sprintf(wrinfo, "%s", inet_ntoa(((struct sockaddr_in *)&ifrr.ifr_addr)->sin_addr));
	  
	  // time_t is arithmetic time type
	  time_t now;
	  
	  // Obtain current time
	  // time() returns the current time of the system as a time_t value
	  time(&now);
	  
	  // localtime converts a time_t value to calendar time and 
	  // returns a pointer to a tm structure with its members 
	  // filled with the corresponding values
	  struct tm *local = localtime(&now);
	  
	  
	  // clear lcd
	  LCDclear();
	  
	  // get system usage / info
	  struct sysinfo sys_info;
	  if(sysinfo(&sys_info) != 0)
	  {
		printf("sysinfo-Error\n");
	  }
	  
	  
	  // time info
	  char timeInfo[10]; 
	  unsigned long seconds = local->tm_sec;
	  unsigned long minutes = local->tm_min;
	  unsigned long hours = local->tm_hour;
	  sprintf(timeInfo, "      %02d:%02d:%02d", hours, minutes, seconds);
	  
	  // uptime
	  char uptimeInfo[15];
	  unsigned long uph = sys_info.uptime / 3600;
	  unsigned long upm = (sys_info.uptime / 60) - (uph * 60);
	  unsigned long ups = sys_info.uptime - (upm * 60) - (uph * 3600);
	  sprintf(uptimeInfo, "Up    %02d:%02d:%02d", uph, upm, ups);
	  
	  // cpu info
	  char cpuInfo[10]; 
	  unsigned long avgCpuLoad = sys_info.loads[0] / 4000;
	  sprintf(cpuInfo, "CPU %ld%%", avgCpuLoad);
	  
	  // ram info
	  char ramInfo[10]; 
	  unsigned long totalRam = (sys_info.totalram - sys_info.freeram - sys_info.bufferram - sys_info.sharedram) / 1024 / 1024;
	  sprintf(ramInfo, "RAM %ld MB", totalRam);
	  
	  	  
	  // build screen
	  LCDdrawstring(0, 0, timeInfo);
	  //LCDdrawstring(0, 0, "Raspberry Pi:");
	  //LCDdrawline(0, 10, 83, 10, BLACK);
	  LCDdrawstring(0, 8, uptimeInfo);
	  LCDdrawstring(0, 16, cpuInfo);
	  LCDdrawstring(0, 24, ramInfo);
	  LCDdrawstring(0, 32, wlinfo);
	  LCDdrawstring(0, 40, wrinfo);
	  LCDdisplay();
	  
	  delay(1000);
  }
  
    //for (;;){
  //  printf("LED On\n");
  //  digitalWrite(pin, 1);
  //  delay(250);
  //  printf("LED Off\n");
  //  digitalWrite(pin, 0);
  //  delay(250);
  //}

  return 0;
}
