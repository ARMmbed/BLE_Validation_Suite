import serial
import datetime
import time
import sys
import threading
from types import *
from Validation_Suite import *

TIMEOUT = 60

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
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		print outputB,
		if '{{failure}}' in outputB:
			print '\tMBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print '\tMBED[B]: ' + outputB,
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
	if addr1 == addr2:
		return True
	else:
		return False

'''! test to see if the advertisement interval can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''	
def changeIntervalTest(aSer,bSer):
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
			print '\tMBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'Data' in outputB:
			time1 = time.time()
			uptime = time1 - time2
			if approxEqual(uptime,0.5,0.2):
				counter1 = counter1 + 1
				avg1 = avg1 + uptime
			print '\tPC: Time interval: ' + str(uptime)
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
			if time.time() - time3 > TIMEOUT:
				return False

'''! test to see if the advertisement payload can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''		
def changePayloadTest(aSer,bSer):
	counter = 0
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print '\tMBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print '\tMBED[B]: ' + outputB,
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
	counter = 0
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print '\tMBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print '\tMBED[B]: ' + outputB,
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
	bSer.timeout = 3
	counter = 0
	counter2 = 0
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print '\tMBED[B]: ' + outputB,
			return False
		if '{{success}}' not in outputB and outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'Data' in outputB:
			counter = 0
		if outputB == '':
			counter = counter + 1
		if counter2 > 15:
			if counter > 5:
				return True
			else:
				return False
		if time.time() - time1 > TIMEOUT:
			return False
		counter2 = counter2 + 1


def shutdownTest(aSer, bSer):
	stopAd = aSer.readline()
	if '{{failure}}' in stopAd:
		print '\tMBED[A]: ' + stopAd,
		return False
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
	time1 = time.time()
	while True:
		outputB = bSer.readline()
		if '{{failure}}' in outputB:
			print '\tMBED[B]: ' + outputB,
			return False
		if 'Data: 6' in outputB:
			print outputB,
			return True
		if time.time() - time1 > TIMEOUT:
			return False
