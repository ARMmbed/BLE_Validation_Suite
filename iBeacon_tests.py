import serial
import datetime
import time
import sys
import threading
from types import *
from Validation_Suite import *

TIMEOUT = 30

'''! Calculates if inputs are with approximately equal to each other within a range reutrn Boolean
@param x first input
@param y second input
@param range the range tolerance
'''
def approxEqual(x, y, range):
	if abs(x-y) <= range:
		return True
	else:
		return False

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

'''! Test to set and get the MAC address 
@param aSer the serial object for device A
@param bSer the serial object for device B
'''
def setAddrTest(aSer,bSer):
	setAddrError = aSer.readline()
	print setAddrError
	getAddrError = aSer.readline()
	print getAddrError
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
def changeIntervalTest(aSer,bSer):
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
			print 'PC: Time interval: ' + str(uptime)
			time2 = time.time()
			if counter1 == 5:
				avg1 = avg1/counter1
				if approxEqual(avg1,0.5,0.05):
					return True
				else:
					return False
			elif counter3 > 30:
				return False
			counter3 = counter3 + 1
			if time.time() - time3 > TIMEOUT * 1.5:
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


def shutdownTest(aSer, bSer):
	stopAd = aSer.readline()
	if '{{failure}}' in stopAd:
		print 'MBED[A]: ' + stopAd,
	shutdown = aSer.readline()
	if '{{failure}}' in shutdown:
		print 'MBED[A]: ' + shutdown,
	bleinit = aSer.readline()
	if '{{failure}}' in bleinit:
		print 'MBED[A]: ' + bleinit,
	startAd = aSer.readline()
	if '{{failure}}' in startAd:
		print 'MBED[A]: ' + startAd,
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print 'MBED[B]: ' + outputB,
			return False
		if 'Data: 6' in outputB:
			print outputB,
			return True
		if time.time() - time1 > 30:
			return False
