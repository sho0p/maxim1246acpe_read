import wiringpi
SPIchannel = 1 #SPI Channel (CE1)
SPIspeed = 500000 #Clock Speed in Hz
wiringpi.wiringPiSetupGpio()
wiringpi.wiringPiSPISetup(SPIchannel, SPIspeed)

sendData = str(00) #will send TWO bytes, a byte 4 and a byte 2
recvData = wiringpi.wiringPiSPIDataRW(SPIchannel, sendData)
#recvData now holds a list [NumOfBytes, recvDataStr] e.g. [2, '\x9A\xCD']

print(recvData[1])

#alternatively, to send a single byte:
#sendData = chr(42) #will send a single byte containing 42
#recvData = wiringpi.wiringPiSPIDataRW(SPIchannel, sendData)
#recvData is again a list e.g. [1, '\x9A']
