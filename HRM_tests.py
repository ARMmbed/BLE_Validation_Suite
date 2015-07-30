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
def connectTest(aSer,bSer):
	thread = threading.Thread(target = aSerialRead, args = (aSer,'MBED[A]: ',))
	thread.daemon = True
	thread.start()
	time1 = time.time()
	while(True):
		outputB = bSer.readline()
		if 'Devices already connected' in outputB:
			return True
		if '{{success}}' not in outputB and outputB != '':
			print 'MBED[B]: ' + outputB,
		if 'Connected' in outputB:
			return True
		if time.time() - time1 > TIMEOUT:
			return False
		
'''! tests setDeviceName and getDeviceName functions, only runnable when devices are disconnected
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

'''! tests setAppearance and getAppearance functions, only runnable when devices are disconnected
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

'''! tests the get/setPreferredConnectionParams functions, only runnable when devices are disconnected
@param aSer the serial port for device A
@param bSer teh serial port for device B
'''
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
	
'''! test to read device A's HRM characteristic from device B, only runnable when devices are connected
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def readTest(aSer,bSer):
	outputB = bSer.readline()
	if 'Devices must be connected' in outputB:
		print 'device must be connected'
		return False
	if 'not found' in outputB:
		print 'HRM Characteristic not found'
		return False
	outputB = bSer.readline()
	if '{{success}}' not in outputB and outputB != '':
		print 'MBED[B]: ' + outputB,
	if 'HRM' in outputB:
		return True
	else:
		print 'MBED[B]: ' + outputB
		return False

'''! test to write "1" to device A's LED characteristic from device B and read back to verify, only runnable when devices are connected
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def writeTest(aSer,bSer):
	outputB = bSer.readline()
	if 'Devices must be connected' in outputB:
		print 'device must be connected'
		return False
	outputB = bSer.readline()
	if '{{success}}' not in outputB and outputB != '':
		print 'MBED[B]: ' + outputB,
	outputB = bSer.readline()
	if '{{success}}' not in outputB and outputB != '':
		print 'MBED[B]: ' + outputB,
	if 'LED: 1' in outputB:
		return True
	else:
		return False
	

'''! test to disconnect device B and device A, this enables certain tests to be able to run, only runnable when devices are connected
@param aSer the serial object for device A
@param bSer the serial object for device B 
'''
def disconnectTest(aSer,bSer):
	outputB = bSer.readline()
	if '{{success}}' not in outputB:
		print 'MBED[B]: ' + outputB,
		return False
	else:
		return True	