# BLE_Validation_Suite
Validation Suite for BLE_API checking 

# Prerequisites
Python 2.7.x

Install mbed-ls (to get the ability to find connected mbed supported devices and their serial port and mount points)

Install mbedhtrun (to get the ability to flash the device automatically)

Optionally install yotta to use the yotta build system. Or one can use their own built hex files

# Usage

Setup the config.json file. Run in the command line

```
$ python Validation_Suite.py
```

# Config
"test_name" should be HRM, iBeacon or Block depending on which tests you want to run

"description" is the message which is printed at the beginning of the test

"nodes" is the connected boards, "*" indicates a platform which is in both

"build_system" is the system used to build the tests, "yotta" is the recommended build system. Setting "clean" to true
will clean the build path and "build" will build the system. This is useful if you dont want to rebuild the system but 
just flash whatever is already built. 
Using "abs_path" allows the user to flash an already built binary from their own toolchain or the online IDE. 

"skip-flash" set to true will skip the flashing stage and just run the test suite

"timeout" is the timeout before the tests fail. 

# Adding new tests

To add a test for advertising add a test function to iBeacon_tests.py make sure the function name finishes with "Test". 
Add a function to A.cpp and add an if to commandInterpreter() the string compared should be the function name without the "Test".


to add a test for connected devices add a test function to HRM_tests.p make sure the function name finishes with "TestA" or "TestB" depending on where the test is for device A or B. Add a function to A.cpp or B.cpp depending on if the test is in testDictA or testDictB and add an if to the respective commandInterpreter() function in the .cpp file the string compared should be teh function name with "TestA" or "TestB"
