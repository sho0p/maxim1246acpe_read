all:
	gcc adc.c -o adc.bin -lwiringPi -lrt -pthread -lcrypt

clean:
	rm -rf adc.bin
