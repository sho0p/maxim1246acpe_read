import os
import time
import subprocess
import statistics

step = float(input("Input step size: "))
stranl = float(input("Input lower bound of scanned range: "))
stranh = float(input("Input upper bound of scanned range: "))

readstddev = {}
sweep = stranl
lpfb = step
while lpfb < stranh:
	print(lpfb)
	out = subprocess.call(['./adc.bin', '/dev/spidev1.0', str(lpfb), '>', 'test.txt'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
	read_list = []
	with open('test.txt') as fp:
		for line in fp:
			read_list.append(line)
			print(line)
	readstddev[sweep] = statistics.stdev(read_list)
	lpfb=lpfb+step
	
best = min(readstddev, key=readstddev.get)
print("Best lambda in sweep: ", best, " with a std dev of ", readstddev[best])
