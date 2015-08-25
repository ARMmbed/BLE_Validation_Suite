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
	try:
		A = subprocess.check_output(['mbedls','--json'])
		d = json.loads(A)
		return d[devNo][var]
	except IndexError:
		print 'Cannot find device mounted'
		sys.exit()
	
'''! calls shell mbedhtrun which flashes devices specified
@param mount the mount point of device e.g. E:/
@param serial the serial port number e.g. COM3
@param file the name of the file being flashed in the same directory
@param device the device type e.g. NRF51822
'''
def flashDevice(mount,serial,file,device):
	subprocess.call(['mbedhtrun','-d', mount,'-f', file,'-p', serial,'-C', '2', '-c', 'copy', '-m', device, '--run'])
	return 

def checkInit(ser, str):
	result = False
	time1 = time.time()
	while time.time() - time1 < 30:
		output = ser.readline()
		if 'ASSERTIONS DONE' in output:
			result = True
			break
		if '{{success}}' not in output:
			print '\tMBED[{0}]: '.format(str) + output,
			result = True
		if 'Device must be' in output:
			result = None
			break
		if 'not found' in output:
			result = False
			break
		if '{{failure}}' in output:
			result = False
			break
	return result

'''! flushes the serial ports for device A and B
@param aSer the serial object for device A
@param bSer the serial object for device B
'''
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
	result = checkInit(bSer, 'B')
	if result == False:
		print 'Initial requirements failed. Test ending'
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
			result = testDict[i](aSer, bSer)
			if result is None:
				pass
			elif result:
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
			result = funcTest(aSer, bSer)
			if result is None:
				pass
			elif result:
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
	result = checkInit(bSer, 'B')
	if result == False:
		print 'Initial requirements failed. Test ending'
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
			result = testDictA[i](aSer, bSer)
			if result is None:
				pass
			elif result:
				passList = passList + [i]
			else:
				failList = failList + [i]
			time.sleep(2)
		flushSerials(aSer, bSer)
		print '\nRunning connect test\n'
		bSer.write('connect\n')
		result = HRM.connectTestB(aSer, bSer)
		if result is None:
			pass
		elif result:
			passList = passList + ['connect']
		else:
			failList = failList + ['connect']
		time.sleep(2)
		for i in testDictB:
			print '\nRunning ' + str(i) + ' test\n'
			flushSerials(aSer, bSer)
			bSer.write(str(i) + '\n')
			result = testDictB[i](aSer, bSer)
			if result is None:
				pass
			elif result:
				passList = passList + [i]
			else:
				failList = failList + [i]
			time.sleep(2)
		flushSerials(aSer, bSer)
		print '\nRunning disconnect test\n'	
		bSer.write('disconnect\n')
		time.sleep(2)
		result = HRM.disconnectTestB(aSer, bSer)
		if result is None:
			pass
		elif result:
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
			
			result = funcTest(aSer, bSer)
			if result is None:
				pass
			elif result:
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
	print bSer.readline()
	for i in MAC:
		bSer.write(i + '\n')
	print 'BLE MAC written'


'''! builds the files to be flashed using yotta
'''
def yottaBuild():
	try:
		if '-iBeacon' in sys.argv:
			if '-mbedos' in sys.argv:
				os.chdir('Aos')
				subprocess.check_call(['yt', 'build'])
				os.chdir('..')
				os.chdir('Bos')
				subprocess.check_call(['yt', 'build'])
				os.chdir('..')
			else:
				os.chdir('A')
				subprocess.check_call(['yt', 'build'])
				os.chdir('..')
				os.chdir('B')
				subprocess.check_call(['yt', 'build'])
				os.chdir('..')
		elif '-HRM' in sys.argv:
			if '-mbedos' in sys.argv:
				os.chdir('AHRMOS')
				subprocess.check_call(['yt', 'build'])
				os.chdir('..')
				os.chdir('BHRMOS')
				subprocess.check_call(['yt', 'build'])
				os.chdir('..')
			else:
				os.chdir('AHRM')
				subprocess.check_call(['yt', 'build'])
				os.chdir('..')
				os.chdir('BHRM')
				subprocess.check_call(['yt', 'build'])
				os.chdir('..')
		else:
			print 'Invalid test name'
			sys.exit()
	except:
		print sys.exc_info()[0]
		print 'Yotta build failed'
		sys.exit()


'''! gets the file path from the yotta build system
'''
def yottaGetFiles():
	yottaBuild()
	#get the path of the hex file to be copied
	if '-iBeacon' in sys.argv:
		if '-mbedos' in sys.argv:
   			path = [os.path.join(r,name) for r, d, f in os.walk('.') for name in f if 'ble-mbedos-ibeacon' in name if name.endswith("combined.hex")] 
   		else:
   			path = [os.path.join(r,name) for r, d, f in os.walk('.') for name in f if 'ble-ibeacon' in name if name.endswith("combined.hex")]
   	elif '-HRM' in sys.argv:
   		if '-mbedos' in sys.argv:
   			path = [os.path.join(r,name) for r, d, f in os.walk('.') for name in f if 'ble-mbedos-hrm' in name if name.endswith("combined.hex")]
   		else:
   			path = [os.path.join(r,name) for r, d, f in os.walk('.') for name in f if 'ble-hrm' in name if name.endswith("combined.hex")]
   	return path


'''! gets the file name from the user input
'''
def getFiles():
	index = sys.argv.index('-f')
	path = []
	try:
		path.append(sys.argv[index + 1])
		path.append(sys.argv[index + 2])
		if not os.path.exists(path[0]):
			print path[0] + ' does not exist in current directory'
			sys.exit()
		elif not os.path.exists(path[1]):
			print path[1] + ' does not exist in current directory'
			sys.exit()
	except IndexError:
		print 'Two files required'
		sys.exit()
	return path

if __name__ == "__main__":
	# detect the mbed devices connected to the system
	aPort = getJson(0, 'serial_port')
	bPort = getJson(1, 'serial_port')
	aMount = getJson(0, 'mount_point')
	bMount = getJson(1, 'mount_point')
	aName = getJson(0, 'platform_name')
	bName = getJson(1, 'platform_name')


	if len(sys.argv) == 1 or '--help' in sys.argv or '-h' in sys.argv:
		string = ('\nUse (-y) [-mbedos] to specific to build with yotta and if it should be bult for mbedos or mbed classic (mbed classic by default) |\n'
			'Use (-f) (file1) (file2) to specify flashing with your own built binaries \n'
			'(-HRM|-iBeacon) depending on what test you want to build'
			'[i] for interactive mode where you can choose the test')
		print string
		sys.exit()
	elif '-y' in sys.argv:
		path = yottaGetFiles()
	elif '-f' in sys.argv:
		path = getFiles()
	else:
		print 'Error: not enough arguments'
		sys.exit()
	flashDevice(aMount, aPort, path[0], aName)
	flashDevice(bMount, bPort, path[1], bName)
	

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
	elif '-test' in sys.argv:
		iBeaconTest(aSer, bSer)
	else:
		sys.exit()
