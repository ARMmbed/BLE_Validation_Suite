import mbed_lstools
import subprocess
import json
import serial
import datetime
import time
import sys
import threading
import itertools
import types
import os.path
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

def flushSerials(aSer, bSer):
	bSer.flushInput()
	bSer.flushOutput()
	bSer.flush()
	aSer.flushInput()
	aSer.flushOutput()
	aSer.flush()

'''! function to run tests for iBeacon
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def iBeaconTest(aSer, bSer):
	bleInitCheck = bSer.readline()
	if '{{failure}}' in bleInitCheck:
		print 'MBED[B]: ' + bleInitCheck,
		print 'Test cannot continue without ble.init(). Test ending'
		return
	setScanParamsCheck = bSer.readline()
	if '{{failure}}' in setScanParamsCheck:
		print 'MBED[B]: ' + setScanParamsCheck,
		print 'Test cannot continue without setScanParams. Test ending'
		return
	startScanCheck = bSer.readline()
	if '{{failure}}' in startScanCheck:
		print 'MBED[B]: ' + startScanCheck,
		print 'Test cannot continue without startScan. Test ending'		
		return
	passList = []
	failList = []

	# list comprehensions for generating lists of functions and tests from iBeacon_tests.py
	names = [iBeacon.__dict__.get(a).__name__[:-4] for a in dir(iBeacon) if isinstance(iBeacon.__dict__.get(a), types.FunctionType) and 'Test' in iBeacon.__dict__.get(a).__name__]
	funcs = [iBeacon.__dict__.get(a) for a in dir(iBeacon) if isinstance(iBeacon.__dict__.get(a), types.FunctionType) and 'Test' in iBeacon.__dict__.get(a).__name__]

	# dictionary of pairs of function and function names
	testDict = dict(zip(names, funcs))

	if '-i' not in sys.argv:
		for i in testDict:
			time.sleep(4)
			print '\nRunning ' + str(i) + ' test\n'
			flushSerials(aSer, bSer)
			aSer.write(str(i) + '\n')
			if testDict[i](aSer, bSer):
				passList = passList + [i]
			else:
				failList = failList + [i]
			aSer.write('1\n')
	else:
		while True:
			time.sleep(2)
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
			flushSerials(aSer, bSer)
			print ''
			aSer.write(str(testInput) + '\n')
			
			if (funcTest(aSer, bSer)):
				if testInput in failList:
					failList.remove(testInput)
				passList = (list(set(passList + [testInput])))

			else:
				if testInput not in passList:
					failList = (list(set(failList + [testInput])))
			flushSerials(aSer, bSer)
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
def HRMTest(aSer, bSer):
	bleInitCheck = bSer.readline()
	if '{{failure}}' in bleInitCheck:
		print 'MBED[B]: ' + bleInitCheck,
		print 'Test cannot continue with ble.init. Test ending'
		return 
	setScanParamsOutput = bSer.readline()
	if '{{failure}}' in setScanParamsOutput:
		print 'MBED[B]: ' + setScanParamsOutput,
		print 'Test cannot continue without setScanParams. Test ending'
		return 
	passList = []
	failList = []
	bSer.write('1\n')

	# list comprehensions for generating lists of functions and tests from HRM_tests.py
	namesA = [HRM.__dict__.get(a).__name__[:-5] for a in dir(HRM) if isinstance(HRM.__dict__.get(a), types.FunctionType) and 'TestA' in HRM.__dict__.get(a).__name__]
	funcsA = [HRM.__dict__.get(a) for a in dir(HRM) if isinstance(HRM.__dict__.get(a), types.FunctionType) and 'TestA' in HRM.__dict__.get(a).__name__]

	# dictionary of pairs of function and function names
	testDictA = dict(zip(namesA, funcsA))

	namesB = [HRM.__dict__.get(a).__name__[:-5] for a in dir(HRM) if isinstance(HRM.__dict__.get(a), types.FunctionType) and 'TestB' in HRM.__dict__.get(a).__name__]
	funcsB = [HRM.__dict__.get(a) for a in dir(HRM) if isinstance(HRM.__dict__.get(a), types.FunctionType) and 'TestB' in HRM.__dict__.get(a).__name__]

	testDictB = dict(zip(namesB, funcsB))

	if '-i' not in sys.argv:
		del testDictB['connect']
		del testDictB['disconnect']
		for i in testDictA:
			print '\nRunning ' + str(i) + ' test\n'
			flushSerials(aSer, bSer)
			aSer.write(str(i) + '\n')
			if testDictA[i](aSer, bSer):
				passList = passList + [i]
			else:
				failList = failList + [i]
			time.sleep(2)
		flushSerials(aSer, bSer)
		print '\nRunning connect test\n'
		bSer.write('connect\n')
		if HRM.connectTestB(aSer, bSer):
			passList = passList + ['connect']
		else:
			failList = failList + ['connect']
		time.sleep(2)
		for i in testDictB:
			print '\nRunning ' + str(i) + ' test\n'
			flushSerials(aSer, bSer)
			bSer.write(str(i) + '\n')
			if testDictB[i](aSer, bSer):
				passList = passList + [i]
			else:
				failList = failList + [i]
			time.sleep(2)
		flushSerials(aSer, bSer)
		print '\nRunning disconnect test\n'	
		bSer.write('disconnect\n')
		time.sleep(2)
		if HRM.disconnectTestB(aSer, bSer):
			passList = passList + ['disconnect']
		else:
			failList = failList + ['disconnect']
		print ''
	else:
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
			flushSerials(aSer, bSer)
			print ''
			if testInput in testDictA:
				aSer.write(str(testInput) + '\n')
			elif testInput in testDictB:
				bSer.write(str(testInput) + '\n')
			
			if (funcTest(aSer, bSer)):
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
def transferAddr(aSer, bSer):
	aSer.write('1\n')

	errorTest = aSer.readline()
	if '{{success}}' not in errorTest:
		print 'MBED[A]: ' + errorTest,
		sys.exit()
	
	print 'Reading BLE MAC'
	MAC = aSer.readline()
	print 'MAC read'

	MAC = MAC.split(':')
	MAC[-1] = MAC[-1][:-1]
	print 'BLE Address of A: ',
	print MAC
	print 'Writing BLE MAC to device B'
	for i in MAC:
		bSer.write(i + '\n')
		
	print 'BLE MAC written'

if __name__ == "__main__":
	# DETECTION
	
	aPort = getJson(0, 'serial_port')
	bPort = getJson(1, 'serial_port')
	aMount = getJson(0, 'mount_point')
	bMount = getJson(1, 'mount_point')
	aName = getJson(0, 'platform_name')
	bName = getJson(1, 'platform_name')
	'''
	bPort = getJson(0, 'serial_port')
	aPort = getJson(1, 'serial_port')
	bMount = getJson(0, 'mount_point')
	aMount = getJson(1, 'mount_point')
	bName = getJson(0, 'platform_name')
	aName = getJson(1, 'platform_name')
	'''
	if len(sys.argv) < 2:
		print 'Give test name as argument e.g. -iBeacon'
		sys.exit()
	#flashing
	if '-f' in sys.argv:
		index = sys.argv.index('-f')
		try:
			aFile = sys.argv[index + 1]
			bFile = sys.argv[index + 2]
			if not os.path.exists(aFile) or not os.path.exists(bFile):
				print 'File does not exist in current directory'
				sys.exit()
		except IndexError:
			print 'Two arguments required'
			sys.exit()
		flashDevice(aMount, aPort, aFile, aName)
		print ''
		flashDevice(bMount, bPort, bFile, bName)
	else:
		if '-iBeacon' in sys.argv:
			flashDevice(aMount, aPort, 'A_NRF51822.hex', aName)
			print ''
			flashDevice(bMount, bPort, 'B_NRF51822.hex', aName)
		elif '-iBeaconNUC' in sys.argv:
			flashDevice(aMount, aPort, 'A_NUC_NUCLEO_F411RE.bin', aName)#'NUCLEO_F401RE')
			print ''
			flashDevice(bMount, bPort, 'B_NRF51822.hex', bName)
		elif '-HRM' in sys.argv:
			flashDevice(aMount, aPort, 'AHRM_NRF51822.hex', aName)
			print ''
			flashDevice(bMount, bPort, 'BHRM_NRF51822.hex', bName)
		else:
			print 'Invalid test name'
			sys.exit()
		
	#Opens ports for logging 
	print 'Opening serial ports from devices to PC\n'
	port = int(aPort[3:])-1
	aSer = serial.Serial(port)
	port = int(bPort[3:])-1
	bSer = serial.Serial(port,timeout = 5)

	transferAddr(aSer, bSer)

	if '-iBeacon' in sys.argv:
		iBeaconTest(aSer, bSer)
	elif '-HRM' in sys.argv:
		HRMTest(aSer, bSer)
	else:
		sys.exit()
