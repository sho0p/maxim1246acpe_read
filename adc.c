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
float lpf_beta = 0.025;
int failcount;
float prev_dat = 0;

struct spi_ioc_transfer xfer[2];

int spi_init(char filename[40]){
	int file;
	__u8	mode, lsb, bits;
	__u32 	speed = 25;

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
	xfer[0].delay_usecs = 1500;
	xfer[0].speed_hz = 250000;
	xfer[0].bits_per_word = 8;
	xfer[1].cs_change = 0;
	xfer[1].delay_usecs = 1500;
	xfer[1].speed_hz = 250000;
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
	xfer[1].len = 3;
	status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	if (status < 0){
		perror("X_SPI_IOC_MESSAGE");
		return;
	}

	com_serial = 1;
	failcount = 0;
	return retbuf;
}

float lpf(float sig){
	if(prev_dat == 0){
		prev_dat = sig;
		return sig;
	}
	float tmp = sig;
	sig = prev_dat - (lpf_beta * (prev_dat-sig));
	prev_dat = tmp;
	return sig;
}

uint32_t reverse(uint32_t x, int bits){
	x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1); // Swap _<>_
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2); // Swap __<>__
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4); // Swap ____<>____
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8); // Swap ...
    x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16); // Swap ...
    return x >> (32 - bits);
}

void printResults(char * buf){
	uint16_t msg = (buf[0] << 9) | (buf[1]>>3);
	float msg_filt = (float)msg;//(float)reverse((uint32_t)msg, 12);
	msg_filt = lpf(msg_filt);
	//if (msg == 0) return;
	printf("%.2f\n",msg_filt );
	return;
}

void printResultsHex(char * buf){
	printf("0x%X 0x%X 0x%X\n",buf[0], buf[1], buf[2]);
}
int main(int argc, char ** argv){
	int i;
	char wr_buf[] = {TB1, RDMSG, RDMSG};
	char rd_buf[3];;
	if ( argc < 3){
		printf("Usage:\n%s [device] [lpf_beta]\n", argv[0]);
		exit(1);
	}
	wiringPiSetup();
	digitalWrite(CS_PIN, HIGH);
	pinMode(SSTRB_PIN, INPUT);
	fd =  spi_init(argv[1]);
	lpf_beta = atof(argv[2]);
	//fd = open(argv[1], O_RDWR);
	printf("Initial result = %d\n", fd);

	if ( fd <= 0 ) {
		printf("%s: Device %s is not found\n", argv[0], argv[1]);
		exit(1);
	}
	while (1){
		digitalWrite(CS_PIN, LOW);
		char * buf = spi_xfer(fd, wr_buf);
		digitalWrite(CS_PIN, HIGH);
		printResultsHex(buf);
		usleep(1500);
	}
	close(fd);
	return 0;
}
