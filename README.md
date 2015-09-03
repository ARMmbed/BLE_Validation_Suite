# Description
A validation suite to test the BLE_API functionality of devices. 

The tests work by using using mbedls to detect devices, then mbedhtrun is used to flash devices with binaries. It uses a
config file to specify options in the test. 

The device will run a basic initialisation test to make sure that it has the ability to run the rest of the tests. Then there is a python function for each test which collects the output from the devices and verifies to make sure that it is correct.

# Prerequisites
Python 2.7.x

Install mbed-ls (to get the ability to find connected mbed supported devices and their serial port and mount points)

Install mbedhtrun (to get the ability to flash the device automatically)

Optionally install yotta to use the yotta build system. Or one can use their own built hex files


# Installation
Run
```
$ git clone https://github.com/jslater8/BLE_Validation_Suite.git
```
To verify you have cloned it correctly, run
```
$ python Validation_Suite.py --help
```
Make sure there is no errors
# Usage

Setup the config.json file. Use the config.json file included as a base line.

Make sure the devices connected to the host PC are in the platform.json file

If using yotta, set the targets within the test folders (i.e. A and B or AHRM and BHRM or Block).

If you do not have a yotta target you can test using self built binaries from the online IDE. Copy the source cpp files included for each device and test. Look at "build_system" later on in the config section of the README.

Run in the command line

```
$ python Validation_Suite.py
```

The only flag is "-s" which swaps the way the devices detected using mbedls, this is used to flash the program if they are detected the wrong way round from the test suite (this should be fixed to check the device and flash accordingly). 


# Config file
"test_name" should be HRM, iBeacon or Block depending on which tests you want to run

"description" is the message which is printed at the beginning of the test

"nodes" is the connected boards, "*" indicates a platform which is in both

"build_system" is the system used to build the tests, "yotta" is the recommended build system. Setting "clean" to true
will clean the build path and "build" will build the system. This is useful if you dont want to rebuild the system but 
just flash whatever is already built. 
Using "abs_path" allows the user to flash an already built binary from their own toolchain or the online IDE. When building
the binary, use the included main.cpp files from the A and B folders, AHRM and BHRM folders or BLE_BlockTransfer folder. 

"skip-flash" set to true will skip the flashing stage and just run the test suite

"timeout" is the timeout before the tests fail. 

"interactive" set to true will enable the interactive mode. This will allow the user to run each sub-test individually

"__" before a variable means to not include it, it is useful to add for fast switching between build systems

Look to the included config.json to see an example.

# Adding new tests

To add a test for advertising add a test function to iBeacon_tests.py make sure the function name finishes with "Test". 
Add a function to the A main.cpp in the source folder and add the test name and the function name to the DispatchTableEntry in the getTest function.

to add a test for connected devices add a test function to HRM_tests.p make sure the function name finishes with "TestA" or "TestB" depending on where the test is for device A or B. Add a function to A.cpp or B.cpp depending on if the test is in testDictA or testDictB and add the test name and the function name to the DispatchTableEntry in the getTest function in the respective AHRM or BHRM main.cpp in the source folder 

# Possible improvements for the future

Changing the system to use a json file for each individual test, so users can add tests without having to edit the python file.

Setting the target within the system and specified with the yotta object in the config.json

Support for a variable number of boards connect, currently only works with 2 boards.
