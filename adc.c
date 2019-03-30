#include <wiringPi.h>
#include <errno.h>
#include "adc.h"
#include <unistd.h>

static const int CHANNEL = 1;

int main(){
	int fd, result;
	unsigned char buffer[100];
	wiringPiSetup();
	pinMode(SSTRB_PIN, INPUT);
	pinMode(CS_PIN, OUTPUT);
	printf("Initializing\n");
	digitalWrite(CS_PIN, HIGH);
	fd = wiringPiSPISetup(CHANNEL, 500000);

	printf("Init result: %d\n", fd);

	sleep(5);
	digitalWrite(CS_PIN, LOW);
	buffer[0] = TB1;
	int ignore = wiringPiSPIDataRW(CHANNEL, buffer, 1);
	buffer[0] = RDMSG;
	int msb = wiringPiSPIDataRW(CHANNEL, buffer,1);
	int lsb = wiringPiSPIDataRW(CHANNEL, buffer,1);
	digitalWrite(CS_PIN, HIGH);

	printf("Returned: %d\n", (msb << 8) | lsb);
}