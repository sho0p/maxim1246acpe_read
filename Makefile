all:
	g++ adc.cpp -o adc.bin -lwiringPi -lrt -pthread -lcrypt

clean:
	rm -rf adc.bin
