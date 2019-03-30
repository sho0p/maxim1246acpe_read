//#include <iostream>
#include <wiringPi.h>
#include <stdio.h>
#include <fcntl.h>
#include "adc.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

//using namespace std;

int fd;
<<<<<<< HEAD:adc.cpp
extern int com_serial;
unsigned char wr_buf[] = {TB1, RDMSG, RDMSG};
extern int failcount;

struct spi_ioc_transfer xfer[2];

int spi_init(char filename[40]){
	int file;
	__u8	mode, lsb, bits;
	__u32 	speed = 2500000;

	if ( (file = open(filename, O_RDWR)) < 0){
		printf("Failed to open bus.");
		com_serial = 1;
		exit(1);
	}
			if (ioctl(file, SPI_IOC_WR_MODE, &mode)<0)   {
                perror("can't set spi mode");
                return;
                }
        
 
            if (ioctl(file, SPI_IOC_RD_MODE, &mode) < 0)
                {
                perror("SPI rd_mode");
                return;
                }
            if (ioctl(file, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
                {
                perror("SPI rd_lsb_fist");
                return;
                }
        //sunxi supports only 8 bits
        /*
            if (ioctl(file, SPI_IOC_WR_BITS_PER_WORD, (__u8[1]){8})<0)   
                {
                perror("can't set bits per word");
                return;
                }
        */
            if (ioctl(file, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) 
                {
                perror("SPI bits_per_word");
                return;
                }
        /*
            if (ioctl(file, SPI_IOC_WR_MAX_SPEED_HZ, &speed)<0)  
                {
                perror("can't set max speed hz");
                return;
                }
        */
            if (ioctl(file, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) 
                {
                perror("SPI max_speed_hz");
                return;
                }
     
 
    printf("%s: spi mode %d, %d bits %sper word, %d Hz max\n",filename, mode, bits, lsb ? "(lsb first) " : "", speed);
	xfer[0].len = 3;
	xfer[0].cs_change = 0;
	xfer[0].delay_usecs = 50;
	xfer[0].speed_hz = speed;
	xfer[0].bits_per_word = 8;
	xfer[1].len = 3;
	xfer[1].cs_change = 0;
	xfer[1].delay_usecs = 0;
	xfer[1].speed_hz = speed;
	xfer[1].bits_per_word = 8;
	return file;
}

char * spi_xfer(int file){
	char buf[32];
	xfer[0].tx_buf =(unsigned long) wr_buf;
	xfer[0].len = 3;
	xfer[1].rx_buf = (unsigned long) buf;
	xfer[1].len = 3;
	int status = ioctl(file, SPI_IOC_MESSAGE(2), xfer);
	if ( status < 0){
		perror("SPI_IOC_MESSAGE");
		return;
	}
	com_serial = 1;
	failcount = 0;
	return buf;
}
=======
char wr_buf[] = {TB1, RDMSG, RDMSG};
//char wr_buf[] = {0xFF, 0xFF, 0xFF};
>>>>>>> abe72e5b814fab757a9c725e0563c7c8dc31b5df:adc.c

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
<<<<<<< HEAD:adc.cpp
	fd =  spi_init(argv[1]);
	//fd = open(argv[1], O_RDWR);
	printf("Initial result = %d\n", fd);
=======
	wiringPiSetup();
	pinMode(CS_PIN, OUTPUT);
	pinMode(SSTRB_PIN, INPUT);
	digitalWrite(CS_PIN, 1);
	fd = open(argv[1], O_RDWR);
>>>>>>> abe72e5b814fab757a9c725e0563c7c8dc31b5df:adc.c

	if ( fd <= 0 ) {
		printf("%s: Device %s is not found\n", argv[0], argv[1]);
		exit(1);
	}
<<<<<<< HEAD:adc.cpp
	char * buf = spi_xfer(fd);
	for (int i  = 0; i < ARRAY_SIZE(buf); ++i){
		printf("0x%02X ", buf[i]);
=======

	
	digitalWrite(CS_PIN, 0);
	if ( write(fd, wr_buf, ARRAY_SIZE(wr_buf)) != ARRAY_SIZE(wr_buf)){
		perror("Write Error!");
	}
	while ( digitalRead(SSTRB_PIN) );
	
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
>>>>>>> abe72e5b814fab757a9c725e0563c7c8dc31b5df:adc.c
	}
	printf("\n");
	close(fd);
	return 0;
}
