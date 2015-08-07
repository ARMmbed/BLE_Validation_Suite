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
	return abs(x-y) <= range

'''! tests if the iBeacon service works and prints when an advert is detected
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def detectTest(aSer,bSer):
	interval = aSer.readline()
	if '{{failure}}' in interval:
		print '\tMBED[A]: ' + interval,
		return False
	timeout = aSer.readline()
	if '{{failure}}' in timeout:
		print '\tMBED[A]: ' + timeout,
		return False
	startAdvertisingErrorCode = aSer.readline()
	if '{{failure}}' in startAdvertisingErrorCode:
		print '\tMBED[A]: ' + startAdvertisingErrorCode,
		return False
	counter = 0
	counter2 = 0
	startTime = time.time()
	result = False
	while time.time() - startTime < TIMEOUT:
		outputB = bSer.readline()
		if outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'Data' in outputB:
			result = True
			break
	return result

'''! Test to set and get the MAC address 
@param aSer the serial object for device A
@param bSer the serial object for device B
'''
def setAddrTest(aSer,bSer):
	print '\tSetting BLE MAC Address to 110:100:100:100:100:100'
	setAddrError = aSer.readline()
	if '{{failure}}' in setAddrError:
		print '\tMBED[A]: ' + setAddrError,
		return False
	getAddrError = aSer.readline()
	if '{{failure}}' in getAddrError:
		print '\tMBED[A]: ' + getAddrError,
		return False
	addr1 = aSer.readline()
	addr2 = aSer.readline()
	print '\tMBED[B]: BLE MAC Address: ' + addr2
	return addr1 == addr2

'''! test to see if the advertisement interval can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''	
def changeIntervalTest(aSer,bSer):
	startError = aSer.readline()
	if '{{failure}}' in startError:
		print 'MBED[A]: ' + startError,
		return False
	startError = aSer.readline()
	if '{{failure}}' in startError:
		print 'MBED[A]: ' + startError,
		return False
	counter1 = 0
	avg1 = 0
	time2 = time.time()

	startTime= time.time()
	result = False
	while time.time() - startTime < TIMEOUT:
		outputB = bSer.readline()
		if outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'Data' in outputB and 'ScanResp: 1' not in outputB:
			time1 = time.time()
			uptime = time1 - time2
			if approxEqual(uptime,0.5,0.2):
				counter1 = counter1 + 1
				avg1 = avg1 + uptime
			print '\tPC: Time interval: ' + str(uptime)
			time2 = time.time()
			if counter1 == 5:
				avg1 = avg1/counter1
				result = approxEqual(avg1,0.5,0.05)
				break
	return result


'''! test to see if the advertisement payload can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''		
def changePayloadTest(aSer,bSer):
	for i in xrange(5):
		payloadError = aSer.readline()
		if '{{failure}}' in payloadError:
			print 'MBED[A]: ' + payloadError,
			return False
	startTime = time.time()
	result = False
	while time.time() - startTime < TIMEOUT:
		outputB = bSer.readline()
		if outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'Data: 123' in outputB:
			result = True
			break
	return result
		
'''! test to see if the scan response can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''					
def responseTest(aSer,bSer):
	print '\tSetting scan response'
	payloadError = aSer.readline()
	if '{{failure}}' in payloadError:
		print 'MBED[A]: ' + payloadError,
		return False
	scanError = aSer.readline()
	if '{{failure}}' in scanError:
		print 'MBED[A]: ' + scanError,
		return False
	startAd = aSer.readline()
	if '{{failure}}' in startAd:
		print 'MBED[A]: ' + startAd,
		return False
	counter = 0
	startTime = time.time()
	result = False
	while time.time() - startTime < TIMEOUT:
		outputB = bSer.readline()
		if outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'ScanResp: 1' in outputB:
			result = True
			break
	return result

'''! test to see if the timeout setting can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B
'''					
def setTimeoutTest(aSer,bSer):
	print '\tSetting timeout to 5 seconds'
	startAd = aSer.readline()
	if '{{failure}}' in startAd:
		print 'MBED[A]: ' + startAd,
		return False
	counter = 0
	startTime = time.time()
	result = False
	while time.time() - startTime < TIMEOUT:
		outputB = bSer.readline()
		if outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'Data' in outputB:
			counter = 0
		if outputB == '':
			counter = counter + 1
		if counter > 5:
			print 'test'
			result = True
	print counter
	return result


def shutdownTest(aSer, bSer):
	shutdown = aSer.readline()
	if '{{failure}}' in shutdown:
		print '\tMBED[A]: ' + shutdown,
		return False
	bleinit = aSer.readline()
	if '{{failure}}' in bleinit:
		print '\tMBED[A]: ' + bleinit,
		return False
	startAd = aSer.readline()
	if '{{failure}}' in startAd:
		print '\tMBED[A]: ' + startAd,
		return False

	startTime = time.time()
	result = False
	while time.time() - startTime < TIMEOUT:
		outputB = bSer.readline()
		if outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'Data: 6' in outputB:
			result = True
			break
	return result
