import serial
import datetime
import time
import sys
import threading
from types import *
from Validation_Suite import *

	
'''! calls shell mbedhtrun which flashes devices specified
@param mount the mount point of device e.g. E:/
@param serial the serial port number e.g. COM3
@param file the name of the file being flashed in the same directory
@param device the device type e.g. NRF51822
'''
def flashDevice(mount,serial,file,device):
	subprocess.call(['mbedhtrun','-d', mount,'-f', file,'-p', serial,'-C', '2', '-c', 'copy', '-m', device, '--run'])
	return 

'''! tests if the iBeacon service works and prints when an advert is detected
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def detectTest(aSer,bSer):
	interval = aSer.readline()
	if '{{failure}}' in interval:
		print 'MBED[A]: ' + interval,
	timeout = aSer.readline()
	if '{{failure}}' in timeout:
		print 'MBED[A]: ' + timeout,
	startAdvertisingErrorCode = aSer.readline()
	if '{{failure}}' in startAdvertisingErrorCode:
		print 'MBED[A]: ' + startAdvertisingErrorCode,

	print '\nTESTING SENDING ADVERTISEMENTS\n'
	counter = 0
	counter2 = 0
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		print outputB,
		if '{{failure}}' in outputB:
			print 'MBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print 'MBED[B]: ' + outputB,
		if 'Data' in outputB:
			counter = counter + 1
		if counter == 5:
			return True
		if counter2 > 50:
			return False
		if (time.time() - time1 > TIMEOUT):
			return False

def setAddrTest(aSer,bSer):
	setAddrError = aSer.readline()
	getAddrError = aSer.readline()
	Pass = True
	if '{{failure}}' in setAddrError:
		print 'MBED[A]: ' + setAddrError,
		return False
	if '{{failure}}' in getAddrError:
		print 'MBED[A]: ' + getAddrError,
		return False
	addr1 = aSer.readline()
	addr2 = aSer.readline()
	if addr1 == addr2:
		return True
	else:
		return False

'''! test to see if the advertisement interval can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''	
def changeAdTest(aSer,bSer):
	print '\nTESTING setAdvertisingInterval\n'
	counter1 = 0
	counter2 = 0
	counter3 = 0
	avg1 = 0
	avg2 = 0 
	avg3 = 0 
	time2 = time.time()
	time3 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print 'MBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print 'MBED[B]: ' + outputB,
		if 'Data' in outputB:
			time1 = time.time()
			uptime = time1 - time2
			if approxEqual(uptime,0.5,0.2):
				counter1 = counter1 + 1
				avg1 = avg1 + uptime
			#elif approxEqual(uptime,1,0.2):
			#	counter2 = counter2 + 1
			#	avg2 = avg2 + uptime
			print 'PC: Time interval: ' + str(uptime)
			time2 = time.time()
			if counter1 == 5:
				avg1 = avg1/counter1
				#avg2 = avg2/counter2
				if approxEqual(avg1,0.5,0.05):# and approxEqual(avg2,1.0,0.05):
					return True
				else:
					return False
			elif counter3 > 30:
				return False
			counter3 = counter3 + 1
			if time.time() - time3 > TIMEOUT:
				return False

'''! test to see if the advertisement payload can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''		
def changePayloadTest(aSer,bSer):
	print '\nTESTING accumulateAdvertisingPayload\n'
	counter = 0
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print 'MBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print 'MBED[B]: ' + outputB,
		if 'Data: 123' in outputB:
			return True
		if counter == 15:
			return False
		if time.time() - time1 > TIMEOUT:
			return False

'''! test to see if the scan response can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''					
def responseTest(aSer,bSer):
	print '\nTESTING accumulateScanResponse\n'
	counter = 0
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print 'MBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print 'MBED[B]: ' + outputB,
		if 'ScanResp: 1' in outputB:
			return True
		if 'ScanResp: 0' in outputB:
			counter = counter + 1
		if counter > 30:
			return False
		if time.time() - time1 > TIMEOUT:
			return False

'''! test to see if the timeout setting can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''					
def setTimeoutTest(aSer,bSer):
	print '\nTESTING setAdvertisingTimeout\n'
	bSer.timeout = 3
	counter = 0
	counter2 = 0
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print 'MBED[B]: ' + outputB,
			break
		if '{{success}}' not in outputB and outputB != '':
			print 'MBED[B]: ' + outputB,
		if 'Data' in outputB:
			counter = 0
		if outputB == '':
			counter = counter + 1
		if counter2 > 15:
			if counter > 5:
				return True
			else:
				return False
		if time.time() - time1 > TIMEOUT*2:
			return False
		counter2 = counter2 + 1
