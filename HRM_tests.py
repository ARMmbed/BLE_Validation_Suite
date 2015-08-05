import serial
import datetime
import time
import sys
import threading
from types import *
from Validation_Suite import *

TIMEOUT = 30

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

'''! test function to monitor time between advertisements, enables certain tests to be able to run, only runnable when devices are disconnected
@param aSer serial port of device A
@param bSer serial port of device B
'''
def connectTestB(aSer,bSer):
	thread = threading.Thread(target = aSerialRead, args = (aSer,'MBED[A]: ',))
	thread.daemon = True
	thread.start()
	time1 = time.time()
	while(True):
		outputB = bSer.readline()
		if 'Devices already connected' in outputB:
			return True
		if '{{success}}' not in outputB and outputB != '':
			print '\tMBED[B]: ' + outputB,
		if 'Connected' in outputB:
			time.sleep(2)
			return True
		if time.time() - time1 > TIMEOUT:
			return False
		
'''! tests setDeviceName and getDeviceName functions, only runnable when devices are disconnected
@param aSer the serial port for device A
'''
def setDeviceNameTestA(aSer,bSer):
	print '\tSetting Device name to Josh-test\n'
	outputB = aSer.readline()
	if 'Device must be disconnected' in outputB:
		print '\tDevice must be disconnected'
		return False
	if '{{success}}' not in outputB:
		print '\tMBED[B]: ' + outputB,
	if '{{failure}}' in outputB:
		return False
	outputB = aSer.readline()
	if '{{success}}' not in outputB:
		print '\tMBED[B]: ' +outputB,
	if '{{failure}}' in outputB:
		return False
	deviceName = aSer.readline()
	deviceNameIn = aSer.readline()
	print '\tMBED[B]: Device name: ' + deviceName,
	if deviceName == deviceNameIn:
		return True
	else:
		return False

'''! tests setAppearance and getAppearance functions, only runnable when devices are disconnected
@param aSer the serial port for device A
'''
def appearanceTestA(aSer,bSer):
	print '\tSetting appearance to GENERIC_PHONE (64)\n'
	
	outputB = aSer.readline()
	if 'Device must be disconnected' in outputB:
		print '\tDevice must be disconnected'
		return False
	if '{{success}}' not in outputB:
		print outputB,
	if '{{failure}}' in outputB:
		return False
	outputB = aSer.readline()
	if '{{success}}' not in outputB:
		print outputB,
	if '{{failure}}' in outputB:
		return False
	appearance = aSer.readline()
	print '\tMBED[B]: Appearance = ' + appearance,
	if '64' in appearance:
		return True
	else:
		print outputB
		return False

'''! tests the get/setPreferredConnectionParams functions, only runnable when devices are disconnected
@param aSer the serial port for device A
@param bSer teh serial port for device B
'''
def connParamTestA(aSer,bSer):
	getConn = aSer.readline()
	if '{{failure}}' in getConn:
		return False
	if 'Device must be disconnected' in getConn:
		print '\tDevice must be disconnected'
		return False
	print '\tSetting minConnectionInterval to 50'
	print '\tSetting maxConnectionInterval to 500'
	print '\tSetting slave to 0'
	print '\tSetting connectionSupervisionTimeout to 500\n'
	setConn = aSer.readline()
	if '{{success}}' not in getConn:
		print getConn,
	if '{{success}}' not in setConn:
		print setConn,
	if '{{failure}}' in setConn:
		return False
	minConn = aSer.readline()
	if '{{failure}}' in minConn:
		print '\tMBED[A]: ' + minConn,
		return False
	maxConn = aSer.readline()
	if '{{failure}}' in maxConn:
		print '\tMBED[A]: ' + maxConn,
		return False
	slave = aSer.readline()
	if '{{failure}}' in slave:
		print '\tMBED[A]: ' + slave,
		return False
	connSup = aSer.readline()
	if '{{failure}}' in connSup:
		print '\tMBED[A]: ' + connSup,
		return False
	print '\tminConnectionInterval: ' + minConn,
	print '\tmaxConnectionInterval: ' + maxConn,
	print '\tslave: ' + slave,
	print '\tconnectionSupervisionTimeout: ' + connSup,
	if '50' in minConn and '500' in maxConn and '0' in slave and '500' in connSup:
		return True
	else:
		return False
	
'''! test to read device A's HRM characteristic from device B, only runnable when devices are connected
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def readTestB(aSer,bSer):
	print 'Reading HRM characteristic\n'
	outputB = bSer.readline()
	if 'Devices must be connected' in outputB:
		print '\tDevice must be connected'
		return False
	if 'not found' in outputB:
		print '\tHRM Characteristic not found'
		return False
	if '{{failure}}' in outputB:
		print '\tMBED[B]: ' + outputB,
		return False
	outputB = bSer.readline()
	if '{{success}}' not in outputB and outputB != '':
		print '\tMBED[B]: ' + outputB,
	if 'HRM' in outputB:
		return True
	else:
		print '\tMBED[B]: ' + outputB
		return False

'''! test to write "1" to device A's LED characteristic from device B and read back to verify, only runnable when devices are connected
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def writeTestB(aSer,bSer):
	print 'Writing 1 to LED characteristic\n'
	outputB = bSer.readline()
	if 'Devices must be connected' in outputB:
		print '\tDevice must be connected'
		return False
	outputB = bSer.readline()
	if '{{success}}' not in outputB and outputB != '':
		print '\tMBED[B]: ' + outputB,
	if '{{failure}}' in outputB:
		return False
	outputB = bSer.readline()
	if '{{success}}' not in outputB and outputB != '':
		print '\tMBED[B]: ' + outputB,
	if '{{failure}}' in outputB:
		return False
	if 'LED: 1' in outputB:
		return True
	else:
		return False

'''!
@param aSer the serial object for device A
@param bSer the serial object for device B
'''	
def notificationTestB(aSer, bSer):
	print 'Enabling notifications\n'
	writeError = bSer.readline()
	if '{{failure}}' in writeError:
		print '\tMBED[B]: ' + writeError,
		return False
	if 'Devices must be connected' in writeError:
		print '\tDevice must be connected'
		return False
	Sync = bSer.readline()
	print '\tChanging button characteristic to 1'
	aSer.write('notification\n')
	hvxCallback = bSer.readline()
	print '\tMBED[B]: ' + hvxCallback
	if 'Button' not in hvxCallback:
		return False
	return True

'''! test to disconnect device B and device A, this enables certain tests to be able to run, only runnable when devices are connected
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def disconnectTestB(aSer,bSer):
	outputB = bSer.readline()
	if '{{success}}' not in outputB:
		print '\tMBED[B]: ' + outputB,
		return False
	else:
		return True	