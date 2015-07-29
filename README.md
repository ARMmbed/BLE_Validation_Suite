# BLE_Validation_Suite
Validation Suite for BLE_API checking 

# Prerequisites
Python 2.7.x

Install mbed-ls (to get the ability to find connected mbed supported devices and their serial port and mount points)

Install mbedhtrun (to get the ability to flash the device automatically)

# Usage
Generate the .hex or .bin files from the cpp files** for your specific board using whatever toolchain you prefer.
The suggested toolchain would be the mbed online compiler

Make sure the hex files and the python files are in the same directory

run $ python Validation_Suite.py -HRM

or run $ python Validation_Suite.py -iBeacon

to test HRM or iBeacon BLE Capibilities

** A.cpp, B.cpp are used for the iBeacon tests for device A and B. 
AHRM.cpp and BHRM.cpp are used for the HRM tests for device A and B

# Adding new tests

To add a test for advertising add a test function to iBeacon_tests.py. In Validation_Suite.py add the test name string 
and the function to testDict in the iBeaconTest function. Add a function to A.cpp and add an if to commandInterpreter().


to add a test for connected devices add a test function to HRM_tests.py. In Validation_Suite.py add the test name string 
and the function to either testDictA or testDictB. Testing characteristics and services should be put in testDictB and
other in testDictA. Add a function to A.cpp or B.cpp depending on if the test is in testDictA or testDictB and add
an if to the respective commandInterpreter() function in the .cpp file
