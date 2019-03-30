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
                return -1;
                }
        
 
            if (ioctl(file, SPI_IOC_RD_MODE, &mode) < 0)
                {
                perror("SPI rd_mode");
                return -1;
                }
            if (ioctl(file, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
                {
                perror("SPI rd_lsb_fist");
                return -1;
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
                return -1;
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
                return -1;
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
		return buf;
	}
	com_serial = 1;
	failcount = 0;
	return buf;
}

int main(int argc, char ** argv){
	int i;
	char wr_buf[] = {TB1, RDMSG, RDMSG};
	char rd_buf[3];;
	if ( argc < 2){
		printf("Usage:\n%s [device]\n", argv[0]);
		exit(1);
	}
	fd =  spi_init(argv[1]);
	//fd = open(argv[1], O_RDWR);
	printf("Initial result = %d\n", fd);

	if ( fd <= 0 ) {
		printf("%s: Device %s is not found\n", argv[0], argv[1]);
		exit(1);
	}
	char * buf = spi_xfer(fd);
	for (int i  = 0; i < ARRAY_SIZE(buf); ++i){
		printf("0x%02X ", buf[i]);
	}
	printf("\n");
	close(fd);
	return 0;
}
