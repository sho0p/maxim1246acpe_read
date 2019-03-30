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
int com_serial;
unsigned char wr_buf[] = {TB1, RDMSG, RDMSG};
int failcount;

struct spi_ioc_transfer xfer[2];

int spi_init(char filename[40]){
	int file;
	__u8	mode, lsb, bits;
	__u32 	speed = 25000;

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
	xfer[0].cs_change = 0;
	xfer[0].delay_usecs = 10;
	xfer[0].speed_hz = speed;
	xfer[0].bits_per_word = 8;
	xfer[1].cs_change = 0;
	xfer[1].delay_usecs = 10;
	xfer[1].speed_hz = speed;
	xfer[1].bits_per_word = 8;
	return file;
}

char * spi_read(int file){
	digitalWrite(CS_PIN, LOW);
	char buf[16];
	xfer[0].tx_buf =(unsigned long) {0x00, 0x00};
	xfer[0].len = 2;
	xfer[1].rx_buf = (unsigned long) buf;
	xfer[1].len = 16; 
	int status = ioctl(file, SPI_IOC_MESSAGE(2), xfer);
	digitalWrite(CS_PIN, HIGH);
	if ( status < 0){
		perror("R_SPI_IOC_MESSAGE");
		return buf;
	}
	com_serial = 1;
	failcount = 0;
	return buf;
}

void spi_write(int fd, char * msg){
	digitalWrite(CS_PIN, LOW);
	xfer[0].tx_buf = (unsigned long) &msg;
	xfer[0].len = ARRAY_SIZE(msg);
	int status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	digitalWrite(CS_PIN, HIGH);
	if(status < 0){
		perror("W_SPI_IOC_MESSAGE");
		return;
	}
	com_serial = 1;
	failcount = 0;
	return;
}

char * spi_xfer(int fd, char * msg){
	int status;
	static char buf[24], retbuf[24];
//	memset(buf, 0, sizeof(buf));
	buf[0] = msg[0];
	buf[1] = msg[1];
	buf[2] = msg[2];
	xfer[0].tx_buf = (unsigned long) buf;
	xfer[0].len = 1;
	xfer[1].rx_buf = (unsigned long) retbuf;
	xfer[1].len = 16;
	status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	if (status < 0){
		perror("X_SPI_IOC_MESSAGE");
		return;
	}

	com_serial = 1;
	failcount = 0;
	return retbuf;
}
void printResults(char * buf){
	uint16_t msg = (buf[1] << 8) | buf[2];
	printf("%ld\n", (msg >> 4) );
}
void printResultsHex(char * buf){
	printf("0x");
}
int main(int argc, char ** argv){
	int i;
	char wr_buf[] = {TB1, RDMSG, RDMSG};
	char rd_buf[3];;
	if ( argc < 2){
		printf("Usage:\n%s [device]\n", argv[0]);
		exit(1);
	}
	wiringPiSetup();
	digitalWrite(CS_PIN, HIGH);
	pinMode(SSTRB_PIN, INPUT);
	fd =  spi_init(argv[1]);
	//fd = open(argv[1], O_RDWR);
	printf("Initial result = %d\n", fd);

	if ( fd <= 0 ) {
		printf("%s: Device %s is not found\n", argv[0], argv[1]);
		exit(1);
	}
	while(1){
		digitalWrite(CS_PIN, LOW);
		char * buf = spi_xfer(fd, wr_buf);
		digitalWrite(CS_PIN, HIGH);
		printResults(buf);
		usleep(150);
	}
	close(fd);
	return 0;
}
