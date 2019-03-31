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
float prev_dat[4] = {0, 0, 0, 0};

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
	xfer[0].delay_usecs = 10;
	xfer[0].speed_hz = 250000;
	xfer[0].bits_per_word = 8;
	xfer[1].cs_change = 0;
	xfer[1].delay_usecs = 10;
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

static char * spi_xfer(int fd, char * msg){
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

float lpf(int ind, float sig){
	if(prev_dat[ind] == 0){
		prev_dat[ind] = sig;
		return sig;
	}
	float tmp = sig;
	sig = prev_dat[ind] - (lpf_beta * (prev_dat[ind]-sig));
	prev_dat[ind] = tmp;
	return sig;
}

void printResults(int ind, char * buf){
	uint16_t msg = (buf[0] << 5) | (buf[1]>>3);
	float msg_filt = (float)msg;//(float)reverse((uint32_t)msg, 12);
	//msg_filt = lpf(ind,msg_filt);
	//if (msg == 0) return;
	printf("%.2f, ",msg_filt );
	return;
}

void printResultsHex(char * buf){
	printf("0x%X 0x%X 0x%X, ",buf[0], buf[1], buf[2]);
}

void printResults4ch(char * buf[]){
	for (int i = 0; i < 4; i++){
		uint16_t  msg = (buf[i][0] << 5) | (buf[i][1] >> 3);
		float msg_filt = (float)msg;
		msg_filt = lpf(i,msg_filt);
		printf("%.2f ",msg_filt);
	}
	printf("\n");
	return;
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
	char wr_msg[] = {TB1, TB2, TB3, TB4};
	while (1){
		for(i = 0; i < 4; i++){
			wr_buf[0] = wr_msg[i];
			digitalWrite(CS_PIN, LOW);
			char * buf = spi_xfer(fd, wr_buf);
			digitalWrite(CS_PIN, HIGH);
			printResultsHex(i,buf);
		}
		printf("\n");
		// wr_buf[0] = TB1;
		// digitalWrite(CS_PIN, LOW);
		// printf("segfaulting he");
		// char * buf1 = spi_xfer(fd, wr_buf);
		// memset(buf1, 0, sizeof(buf1));
		// digitalWrite(CS_PIN, HIGH);
		// wr_buf[0] = TB2;
		// digitalWrite(CS_PIN, LOW);
		// char * buf2 = spi_xfer(fd, wr_buf);
		// digitalWrite(CS_PIN, HIGH);
		// memset(buf2, 0, sizeof(buf2));
		// wr_buf[0] = TB3;
		// digitalWrite(CS_PIN, LOW);
		// char * buf3 = spi_xfer(fd, wr_buf);
		// digitalWrite(CS_PIN, HIGH);
		// memset(buf3, 0, sizeof(buf3));
		// wr_buf[0] = TB4;
		// digitalWrite(CS_PIN, LOW);
		// char * buf4 = spi_xfer(fd, wr_buf);
		// digitalWrite(CS_PIN, HIGH);
		// memset(buf4, 0, sizeof(buf4));
		// printf("please for the love of god please");
		// printResults4ch(buf1, buf2, buf3, buf4);
	}
	close(fd);
	return 0;
}
