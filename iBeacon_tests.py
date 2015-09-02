import serial
import datetime
import time
import sys
import threading
import json
from types import *
import Validation_Suite as suite

with open('config.json') as json_file:
	config = json.load(json_file)

TIMEOUT = config['timeout']

'''! Calculates if inputs are with approximately equal to each other within a range reutrn Boolean
@param x first input
@param y second input
@param range the range tolerance
'''
def approxEqual(x, y, range):
	return abs(x-y) <= range

'''! tests if the iBeacon service works on A and prints when an advert is detected on the B side
@param aSer the serial object for device A
@param bSer the serial object for device B
'''
def detectTest(aSer,bSer):
	print '\tSetting up iBeacon service'
	result = suite.checkInit(aSer, 'A')
	if result:
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
	result = suite.checkInit(aSer, 'A')
	addr1 = aSer.readline()
	addr2 = aSer.readline()
	print '\tMBED[B]: BLE MAC Address: ' + addr2
	return addr1 == addr2

'''! test to see if the advertisement interval can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''	
def changeIntervalTest(aSer,bSer):
	print '\tSetting advertising interval to 0.5 seconds'
	result = suite.checkInit(aSer, 'A')
	if result:
		PASS_COUNTER = 5
		counter = 0
		avg = 0
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
					counter = counter + 1
					avg = avg + uptime
				print '\tPC: Time interval: ' + str(uptime)
				time2 = time.time()
				if counter == PASS_COUNTER:
					avg = avg/counter
					result = approxEqual(avg,0.5,0.05)
					break
	return result


'''! test to see if the advertisement payload can be changed
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''		
def changePayloadTest(aSer,bSer):
	result = suite.checkInit(aSer, 'A')
	if result:
		result = False
		startTime = time.time()
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
	result = suite.checkInit(aSer, 'A')
	if result:
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
	result = suite.checkInit(aSer, 'A')
	if result:
		PASS_COUNTER = 3
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
			if counter > PASS_COUNTER:
				result = True
				break
	return result


def shutdownTest(aSer, bSer):
	print '\tShutting down ble services and reinitialising, may take some time'
	result = suite.checkInit(aSer, 'A')
	if result:
		startTime = time.time()
		result = False
		while time.time() - startTime < TIMEOUT*2:
			outputB = bSer.readline()
			if outputB != '':
				print '\tMBED[B]: ' + outputB,
			if 'Data:' in outputB:
				result = True
				break
	return result
