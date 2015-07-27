import mbed_lstools
import subprocess
import json
import serial
import datetime
import time
import sys
import threading
from types import *
from pprint import pprint

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
'''! uses mbedls, parses json output and returns object which is specified
@param devNo the device number 0,1,2... up to number of devices plugged in and detected with mbedls
@param var the object name which data is required
'''
def getJson(devNo, var):
	A = subprocess.check_output(['mbedls','--json'])
	d = json.loads(A)
	return d[devNo][var]
	
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

'''! function to run tests for iBeacon
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def iBeaconTest(aSer,bSer):
	bleInitCheck = bSer.readline()
	if '{{failure}}' in bleInitCheck:
		print 'MBED[B]: ' + bleInitCheck,
		print 'Test cannot continue without ble.init(). Test ending'
	setScanParamsCheck = bSer.readline()
	if '{{failure}}' in setScanParamsCheck:
		print 'MBED[B]: ' + setScanParamsCheck,
		print 'Test cannot continue without ble.init(). Test ending'
	startScanCheck = bSer.readline()
	if '{{failure}}' in startScanCheck:
		print 'MBED[B]: ' + startScanCheck,
		print 'Test cannot continue without ble.init(). Test ending'		

	passList = []
	failList = []

	testDict = {'setAdvertisingInterval': changeAdTest, 'accumulateAdvertisingPayload': changePayloadTest, 'setAdvertisingTimeout': setTimeoutTest, \
				'accumulateScanResponse': responseTest, 'iBeaconTest': detectTest, 'setgetAddress': setAddrTest}
	while True:
		print '\nWhat test? -1 to finish',
		for i in testDict.keys():
			print ', ' + i,
		print ''
		testInput = raw_input()
		if testInput == '-1':
			break
		if testInput in testDict:
			funcTest = testDict[testInput]
		else:
			print 'Invalid test name. Try Again'
			continue
		bSer.flushInput()
		bSer.flushOutput()
		bSer.flush()
		aSer.flushInput()
		aSer.flushOutput()
		aSer.flush()
		print ''
		aSer.write(str(testInput) + '\n')
		try:
			if (funcTest(aSer,bSer)):
				passList = (list(set(passList + [testInput])))
			else:
				failList = (list(set(failList + [testInput])))
		except IndexError:
			print 'Invalid test. Try Again'
		
		aSer.write('1\n')
	print 'SUCCESSFUL TESTS: {0}/{1}'.format(len(passList),len(passList + failList))
	print 'TESTS PASSED: ',
	for i in passList:
			print i + ' ',
	print ''
	print 'TESTS FAILED: ',
	for i in failList:
			print i + ' ',


'''! test function to monitor time between advertisements
@param aSer serial port of device A
@param bSer serial port of device B
'''

def connectTest(aSer,bSer):
	thread = threading.Thread(target = aSerialRead, args = (aSer,'MBED[A]: ',))
	thread.daemon = True
	thread.start()
	time1 = time.time()
	while(True):
		outputB = bSer.readline()
		if 'Devices already connected' in outputB:
			return True
		if '{{success}}' not in outputB:
			print 'MBED[B]: ' + outputB,
		if 'Connected' in outputB:
			return True
		if time.time() - time1 > TIMEOUT:
			return False


'''! tests setDeviceName and getDeviceName functions
@param aSer the serial port for device A
'''
def deviceNameTest(aSer,bSer):
	outputB = aSer.readline()
	if 'Device must be disconnected' in outputB:
		print 'device must be disconnected'
		return False
	if '{{success}}' not in outputB:
		print outputB,
	outputB = aSer.readline()
	if '{{success}}' not in outputB:
		print outputB,
	deviceName = aSer.readline()
	deviceNameIn = aSer.readline()
	if deviceName == deviceNameIn:
		return True
	else:
		return False

'''! tests setAppearance and getAppearance functions
@param aSer the serial port for device A
'''
def appearanceTest(aSer,bSer):
	outputB = aSer.readline()
	if 'Device must be disconnected' in outputB:
		print 'device must be disconnected'
		return False
	if '{{success}}' not in outputB:
		print outputB,
	outputB = aSer.readline()
	if '{{success}}' not in outputB:
		print outputB,
	appearance = aSer.readline()
	if '64' in appearance:
		return True
	else:
		print outputB
		return False

def connParamTest(aSer,bSer):
	getConn = aSer.readline()
	if 'Device must be disconnected' in getConn:
		print 'device must be disconnected'
		return False
	setConn = aSer.readline()
	if '{{success}}' not in getConn:
		print getConn,
	if '{{success}}' not in setConn:
		print setConn,
	minConn = aSer.readline()
	maxConn = aSer.readline()
	slave = aSer.readline()
	connSup = aSer.readline()
	if '{{failure}}' in getConn or '{{failure}}' in setConn:
		return False
	else:
		return True
	if '50' in minconn and '500' in maxconn and '0' in slave and '500' in connSup:
		return True
	else:
		return False
	
'''! test for Heart rate monitor, one device connects and reads characteristic, both devices print value
	calculates difference between ticks and makes sure its constant 
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''

def readTest(aSer,bSer):
	outputB = bSer.readline()
	if 'Devices must be connected' in outputB:
		print 'device must be connected'
		return False
	outputB = bSer.readline()
	if '{{success}}' not in outputB:
		print 'MBED[B]: ' + outputB,
	if 'HRM' in outputB:
		return True
	else:
		print 'MBED[B]: ' + outputB
		return False

def writeTest(aSer,bSer):
	outputB = bSer.readline()
	if 'Devices must be connected' in outputB:
		print 'device must be connected'
		return False
	outputB = bSer.readline()
	if '{{success}}' not in outputB:
		print 'MBED[B]: ' + outputB,
	outputB = bSer.readline()
	if '{{success}}' not in outputB:
		print 'MBED[B]: ' + outputB,
	if 'LED: 1' in outputB:
		return True
	else:
		print 'MBED[B]: ' + outputB
		return False

def disconnectTest(aSer,bSer):
	outputB = bSer.readline()
	if '{{success}}' not in outputB:
		print 'MBED[B]: ' + outputB,
		return False
	else:
		return True	

def HRM(aSer,bSer):
	counter = 0
	Pass = True
	passList = []
	failList = []
	bSer.write('1\n')
	funcList = [deviceNameTest, appearanceTest, connParamTest, connectTest, disconnectTest, readTest, writeTest] # list of functions to be tested
	nameList = ['setDeviceName', 'setAppearance', 'testConnectionParams', 'connect', 'disconnect', 'read characteristics', 'write characteristics'] # list of test names
	testDictA = {'setDeviceName': deviceNameTest, 'setAppearance': appearanceTest, 'setPreferredConnectionParams': connParamTest}
				
	testDictB = {'connect': connectTest, 'disconnect': disconnectTest, 'read': readTest, 'write': writeTest}
	while True:
		time.sleep(2)
		print '\nWhat test? -1 to finish',
		for i in testDictA.keys():
			print ', ' + i,
		for i in testDictB.keys():
			print ', ' + i,
		print ''
		testInput = raw_input()
		if testInput == '-1':
			break
		elif testInput in testDictA:
			funcTest = testDictA[testInput]
		elif testInput in testDictB:
			funcTest = testDictB[testInput]
		else:
			print 'Invalid test name. Try Again'
			continue
		bSer.flushInput()
		bSer.flushOutput()
		bSer.flush()
		aSer.flushInput()
		aSer.flushOutput()
		aSer.flush()
		print ''
		if testInput in testDictA:
			aSer.write(str(testInput) + '\n')
		elif testInput in testDictB:
			bSer.write(str(testInput) + '\n')
		
		if (funcTest(aSer,bSer)):
			if testInput in failList:
				failList.remove(testInput)
			passList = (list(set(passList + [testInput])))

		else:
			if testInput not in passList:
				failList = (list(set(failList + [testInput])))
		

	print 'SUCCESSFUL TESTS: {0}/{1}'.format(len(passList),len(passList + failList))
	print 'TESTS PASSED: ',
	for i in passList:
			print i + ' ',
	print ''
	print 'TESTS FAILED: ',
	for i in failList:
			print i + ' ',

'''! transfers MAC address of device A to device B
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def transferAddr(aSer,bSer):
	aSer.write('1\n')

	print 'Reading MAC'
	MAC = aSer.readline()
	print 'MAC read'

	MAC = MAC.split(':')
	MAC[-1] = MAC[-1][:-1]
	print MAC
	print 'Writing MAC'
	for i in MAC:
		bSer.write(i + '\n')
		
	print 'MAC written'

'''! function be used in seperate thread to monitor another serial port in a seperate thread to stop blocking
@param aSer the serial port to be monitored
@param str string which is output to the console as well at the port input to identify which thread/device
'''
def aSerialRead(aSer, str):
	while True:
		output = aSer.readline()
		if output != '' and '{{success}}' not in output:
			print str + output,
		if 'Connected' in output:
			return
			
if __name__ == "__main__":
	# DETECTION
	aPort = getJson(0, 'serial_port')
	bPort = getJson(1, 'serial_port')
	aMount = getJson(0, 'mount_point')
	bMount = getJson(1, 'mount_point')
	aName = getJson(0, 'platform_name')
	bName = getJson(1, 'platform_name')

	if len(sys.argv) < 2:
		print 'Give test name as argument e.g. -iBeacon'
		sys.exit()
	#flashing
	if sys.argv[1] == '-iBeacon':
		flashDevice(aMount, aPort, 'A_NRF51822.hex', aName)
		flashDevice(bMount, bPort, 'B_NRF51822.hex', aName)
	elif sys.argv[1] == '-iBeaconNUC':
		flashDevice(aMount, aPort, 'A_NUC_NUCLEO_F401RE.bin', aName)#'NUCLEO_F401RE')
		flashDevice(bMount, bPort, 'B_NRF51822.hex', bName)
	elif sys.argv[1] == '-HRM':
		flashDevice(aMount, aPort, 'AHRM_NRF51822.hex', aName)
		flashDevice(bMount, bPort, 'BHRM_NRF51822.hex', bName)
	else:
		print 'invalid test name'
		sys.exit()
		
	#Opens ports for logging 
	print 'Opening Ports'
	port = int(aPort[3:])-1
	aSer = serial.Serial(port)
	port = int(bPort[3:])-1
	bSer = serial.Serial(port,timeout = 5)

	transferAddr(aSer,bSer)
	
	if 'iBeacon' in sys.argv[1]:
		iBeaconTest(aSer,bSer)
	elif sys.argv[1] == '-HRM':
		HRM(aSer, bSer)
	else:
		sys.exit()