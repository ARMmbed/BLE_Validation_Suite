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
import HRM_tests as HRM
import iBeacon_tests as iBeacon


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

	testDict = {'setAdvertisingInterval': iBeacon.changeAdTest, 'accumulateAdvertisingPayload': iBeacon.changePayloadTest, 'setAdvertisingTimeout': iBeacon.setTimeoutTest, \
				'accumulateScanResponse': iBeacon.responseTest, 'iBeaconTest': iBeacon.detectTest, 'setgetAddress': iBeacon.setAddrTest}
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

'''! the test environment for connecting devices and characteristic read/write tests
@param aSer the serial object for device A
@param bSer the serial object for device B
'''
def HRMTest(aSer,bSer):
	counter = 0
	Pass = True
	passList = []
	failList = []
	bSer.write('1\n')
	testDictA = {'setDeviceName': HRM.deviceNameTest, 'setAppearance': HRM.appearanceTest, 'testConnectionParams': HRM.connParamTest}
	testDictB = {'connect': HRM.connectTest, 'disconnect': HRM.disconnectTest, 'read': HRM.readTest, 'write': HRM.writeTest}
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
		flashDevice(aMount, aPort, 'A_NUC_NUCLEO_F411RE.bin', aName)#'NUCLEO_F401RE')
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
		HRMTest(aSer, bSer)
	else:
		sys.exit()