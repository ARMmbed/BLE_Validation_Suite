import mbed_lstools
import subprocess
import json
import serial
import time
import sys
import threading
import types
import os.path
from types import *
import HRM_tests as HRM
import iBeacon_tests as iBeacon

with open('config.json') as json_file:
	config = json.load(json_file)

TIMEOUT = config['timeout']

'''! Checks the nodes in config.json to make sure they are included in the platform.json
'''
def checkNodes():
	platforms = json.load(open('platform.json'))
	A = subprocess.check_output(['mbedls', '--json'])
	mbeds = json.loads(A)
	unSuppPlats = []
	result = True
	for mbed in mbeds:
		if mbed['platform_name'] not in platforms['platforms']:
			unSuppPlats.append(mbed['platform_name'])
			result = False
	if not result:
		print 'Platforms not supported in config file'
		pprint(unSuppPlats)
		sys.exit()




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
def flashDevice(mount,serial,file,device, flash):
	try:
		if flash:
			subprocess.check_call(['mbedhtrun','-d', mount,'-f', file,'-p', serial,'-C', '2', '-c', 'copy', '-m', device, '--run', '--skip-flashing'])
		else:
			subprocess.check_call(['mbedhtrun','-d', mount,'-f', file,'-p', serial,'-C', '2', '-c', 'copy', '-m', device, '--run'])
	except:
		print 'mbedhtrun failed'
		sys.exit()
	return 

def checkInit(ser, str):
	result = False
	time1 = time.time()
	while time.time() - time1 < TIMEOUT/2:
		output = ser.readline()
		if 'ASSERTIONS DONE' in output:
			result = True
			break
		if '{{success}}' not in output and output != '':
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
	if not result:
		print 'Check initial conditions failed'
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

	# if not interactive, loop through running all tests
	if not config['interactive']:
		for i in testDict:
			time.sleep(4) 
			print '\nRunning ' + str(i) + ' test\n'
			flushSerials(aSer, bSer) # Flushes the serials to stop one test from interferring with others
			aSer.write(str(i) + '\n') # Writes the the serial of the device to tell which test to run
			result = testDict[i](aSer, bSer)
			if result is None:
				pass
			elif result:
				print '\nTest passed'
				passList = passList + [i]
			else:
				print '\nTest failed'
				failList = failList + [i]
			aSer.write('1\n') # Write when the test in finished
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
				print '\nTest passed'
				if testInput in failList:
					failList.remove(testInput)
				passList = (list(set(passList + [testInput])))
			else:
				print '\nTest failed'
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

	# list comprehensions for generating lists of functions and tests from HRM_tests.py
	namesA = [HRM.__dict__.get(a).__name__[:-5] for a in dir(HRM) if isinstance(HRM.__dict__.get(a), types.FunctionType) and 'TestA' in HRM.__dict__.get(a).__name__]
	funcsA = [HRM.__dict__.get(a) for a in dir(HRM) if isinstance(HRM.__dict__.get(a), types.FunctionType) and 'TestA' in HRM.__dict__.get(a).__name__]

	# dictionary of pairs of function and function names
	testDictA = dict(zip(namesA, funcsA))

	namesB = [HRM.__dict__.get(a).__name__[:-5] for a in dir(HRM) if isinstance(HRM.__dict__.get(a), types.FunctionType) and 'TestB' in HRM.__dict__.get(a).__name__]
	funcsB = [HRM.__dict__.get(a) for a in dir(HRM) if isinstance(HRM.__dict__.get(a), types.FunctionType) and 'TestB' in HRM.__dict__.get(a).__name__]

	testDictB = dict(zip(namesB, funcsB))

	if not config['interactive']:
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
				print '\nTest passed'
				passList = passList + [i]
			else:
				print '\nTest failed'
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
				print '\nTest passed'
				passList = passList + [i]
			else:
				print '\nTest failed'
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
			print '\nTest passed'
			passList = passList + ['disconnect']
		else:
			print '\nTest failed'
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


'''! checks the block transfer service returns the correct value
'''
def Block(aSer, bSer):
	print '\t Testing block transfer service'
	time.sleep(3)
	aSer.write('start\n')
	startTime = time.time()
	result = False
	while time.time() - startTime < TIMEOUT:
		outputA = aSer.readline()
		print 'MBED[A]: ' + outputA,
		if 'main: buffers match' in outputA:
			result = True
			break
	if result:
		print 'Block transfer successful'
	else:
		print 'Block transfer failed'

'''! transfers MAC address of device A to device B
@param aSer the serial object for device A
@param bSer the serial object for device B
'''
def transferAddr(aSer, bSer):
	# Syncing the device with a write
	aSer.write('1\n')

	# Gets back the information on the if the basic assumptions fail or succeed
	errorTest = aSer.readline()
	if '{{success}}' not in errorTest:
		print 'MBED[A]: ' + errorTest,
		sys.exit()
	
	# get the bluetooth MAC address from device A
	print 'Reading BLE MAC'
	MAC = aSer.readline()
	print 'MAC read'

	# Splits up the MAC string from device A into bytes and writes it to device B
	MAC = MAC.split(':')
	MAC[-1] = MAC[-1][:-1]
	print 'BLE Address of A: ',
	print MAC
	print 'Writing BLE MAC to device B'
	for i in MAC:
		bSer.write(i + '\n')
	print 'BLE MAC written'


'''! builds the files to be flashed using yotta
'''
def yotta(str):
	# Calls the yotta command line on iBeacon, HRM or Block transfer folders
	try:
		if config['test_name'] == 'iBeacon':
			os.chdir('A')
			subprocess.check_call(['yt', str])
			os.chdir('..')
			os.chdir('B')
			subprocess.check_call(['yt', str])
			os.chdir('..')
		elif config['test_name'] == 'HRM':
			os.chdir('AHRM')
			subprocess.check_call(['yt', str])
			os.chdir('..')
			os.chdir('BHRM')
			subprocess.check_call(['yt', str])
			os.chdir('..')
		elif config['test_name'] == 'Block':
			os.chdir('BLE_BlockTransfer')
			subprocess.check_call(['yt', str])
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
def yottaGetFiles(test):
	if config['build_system']['yotta']['clean']:
		yotta('clean')
	if config['build_system']['yotta']['build']:
		yotta('build')

	# get the path of the hex file to be copied
	if test == 'iBeacon':
  		path = [os.path.join(r,name) for r, d, f in os.walk('.') for name in f if 'ble-ibeacon' in name if name.endswith("combined.hex")]
   	elif test == 'HRM':
   		path = [os.path.join(r,name) for r, d, f in os.walk('.') for name in f if 'ble-hrm' in name if name.endswith("combined.hex")]
   	elif test == 'Block':
   		path = [os.path.join(r,name) for r, d, f in os.walk('.') for name in f if 'ble-blocktransfer' in name if name.endswith("combined.hex")]
   	if not path:
   		print 'could not find path'
   		sys.exit()
   	return path


'''! gets the file name from the user input
'''
def getFiles():
	if 'abs_path' not in config['build_system']:
		print 'need abs_path in config file'
		sys.exit()
	try:
		if config['test_name'] == 'Block':
			path = [config['build_system']['abs_path']]
		path = config['build_system']['abs_path']
		if not os.path.exists(path[0]):
			print path[0] + ' does not exist in current directory'
			sys.exit()
		elif not os.path.exists(path[1]):
			print path[1] + ' does not exist in current directory'
			sys.exit()
	except IndexError:
		print '2 files required in config file'
		sys.exit()
	return path


'''! checks the config file for all the needed objects
'''
def checkConfig():
	result = True
	configFail = []
	configList = ['test_name', 'description', 'nodes', 'build_system', 'skip-flash', 'timeout', 'interactive']
	for var in configList:
		if var not in config:
			configFail.append(var)
			result = False
	return (result, configFail)


if __name__ == "__main__":


	if '--help' in sys.argv or '-h' in sys.argv:
		string = ('\nSetup the config file for the test you would like to run. Read the readme for more information')
		print string
		sys.exit()

	#Check the config file for the major needed components
	(result, configFail) = checkConfig()
	if not result:
		print 'config file does not include'
		print configFail
		sys.exit()

	print config['description'].encode()

	# Checks what devices are connected to the PC with mbedls
	checkNodes()

	# detect the mbed devices connected to the system
	if '-s' not in sys.argv:
		aPort = getJson(0, 'serial_port')
		bPort = getJson(1, 'serial_port')
		aMount = getJson(0, 'mount_point')
		bMount = getJson(1, 'mount_point')
		aName = getJson(0, 'platform_name')
		bName = getJson(1, 'platform_name')
	else:
		aPort = getJson(1, 'serial_port')
		bPort = getJson(0, 'serial_port')
		aMount = getJson(1, 'mount_point')
		bMount = getJson(0, 'mount_point')
		aName = getJson(1, 'platform_name')
		bName = getJson(0, 'platform_name')

	test = config['test_name']

	# gets the binaries with yotta or the absolute path
	if 'yotta' in config['build_system']:
		path = yottaGetFiles(test)
	elif 'abs_path' in config['build_system']:
		path = getFiles()
	else:
		print 'need yotta or abs_path in build_system in config'
		sys.exit()

   	# checks how many devices are connected. Flashes them according. 
   	# TODO: do it based on the test being run not the size of the path
	if len(path) == 1:
		print 'FLASHING ' + path[0] + ' TO ' + aName
		flashDevice(aMount, aPort, path[0], aName, config['skip-flash'])
		print 'FLASHING ' + path[0] + ' TO ' + bName
		flashDevice(bMount, bPort, path[0], bName, config['skip-flash'])
	else:
		print 'FLASHING ' + path[0] + ' TO ' + aName
		flashDevice(aMount, aPort, path[0], aName, config['skip-flash'])
		print 'FLASHING ' + path[1] + ' TO ' + bName
		flashDevice(bMount, bPort, path[1], bName, config['skip-flash'])


	# Opens ports for outputing to users
	print 'Opening serial ports from devices to PC\n'
	port = int(aPort[3:])-1
	aSer = serial.Serial(port, timeout = 5)
	port = int(bPort[3:])-1
	bSer = serial.Serial(port, timeout = 5)

	# Runs the test indicated in the config file
	if test == 'iBeacon':
		transferAddr(aSer, bSer)
		iBeaconTest(aSer, bSer)
	elif test == 'HRM':
		transferAddr(aSer, bSer)
		HRMTest(aSer, bSer)
	elif test == 'Block':
		Block(aSer, bSer)
	else:
		sys.exit()

 