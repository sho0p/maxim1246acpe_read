//#include <iostream>
#include <wiringPi.h>
#include <stdio.h>
#include <fcntl.h>
#include "adc.h"
#include <stdlib.h>

//using namespace std;

int fd;
char wr_buf[] = {TB1, RDMSG, RDMSG};
//char wr_buf[] = {0xFF, 0xFF, 0xFF};

int main(int argc, char ** argv){
//	wiringPiSPISetup(0, 500000)

	//wiringPiSPIDataRW(SPI_CH, TB1, 1);
//	wiringPi
	int i;
	char wr_buf[] = {TB1, RDMSG, RDMSG};
	char rd_buf[3];;
	if ( argc < 2){
		printf("Usage:\n%s [device]\n", argv[0]);
		exit(1);
	}
	wiringPiSetup();
	pinMode(CS_PIN, OUTPUT);
	digitalWrite(CS_PIN, 1);
	fd = open(argv[1], O_RDWR);

	if ( fd <= 0 ) {
		printf("%s: Device %s is not found\n", argv[0], argv[1]);
		exit(1);
	}

	
	digitalWrite(CS_PIN, 0);
	if ( write(fd, wr_buf, ARRAY_SIZE(wr_buf)) != ARRAY_SIZE(wr_buf)){
		perror("Write Error!");
	}
	if ( read(fd, rd_buf, ARRAY_SIZE(rd_buf)) != ARRAY_SIZE(rd_buf)){
		perror("Read Error!");
		digitalWrite(CS_PIN, 0);
	}
	else
	{
		digitalWrite(CS_PIN, 0);
		for (i = 0; i < ARRAY_SIZE(rd_buf); i++){
			printf("0x%02X ", rd_buf[i]);
		}
	}
	printf("\n");
	close(fd);
	return 0;
}
