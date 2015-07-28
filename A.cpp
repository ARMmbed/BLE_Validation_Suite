/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "ble/services/iBeacon.h"
#include <string>

#define ASSERT_NO_FAILURE(X) ((X) == BLE_ERROR_NONE) ? (printf("{{success}}\r\n")) : \
                                                        printf("{{failure}} %s at line %u ERROR CODE: %u\r\n", #X, __LINE__, (X));
#define CHECK_EQUALS(X,Y)    ((X) == (Y)) ? (printf("{{sucess}}\n")) : \
                                            (printf("{{failure}} %s != %s at line %u\r\n", #X, #Y, __LINE__));

BLE ble;
DigitalOut myled(LED1);

Gap::AddressType_t addressType;
Gap::Address_t     addrTest = {110,100,100,100,100,100};

void changeAdvPay(void);
void changeScanRes(void);

/**
* Test for advertising using an iBeacon
*/
void setupIBeaconTest(void) {
    /* setup the ibeacon */
    const uint8_t uuid[] = {0xE2, 0x0A, 0x39, 0xF4, 0x73, 0xF5, 0x4B, 0xC4,
                            0xA1, 0x2F, 0x17, 0xD1, 0xAD, 0x07, 0xA9, 0x61 };
    uint16_t majorNumber = 1122;
    uint16_t minorNumber = 3344;
    uint16_t txPower     = 0xC8;
    iBeacon ibeacon(ble, uuid, majorNumber, minorNumber, txPower);

    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    wait(0.5);
    CHECK_EQUALS(ble.gap().getAdvertisingParams().getInterval(), (uint16_t)1000);
    ble.gap().setAdvertisingTimeout(0);
    wait(0.5);
    CHECK_EQUALS(ble.gap().getAdvertisingParams().getTimeout(), 0);
    
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
}
/**
* Test for setting and getting MAC address
*/
void setAddrTest(void){
    Gap::Address_t address;
    Gap::Address_t temp;
    ble.gap().getAddress(&addressType, temp);
    wait(0.2);
    ASSERT_NO_FAILURE(ble.gap().setAddress(Gap::ADDR_TYPE_PUBLIC,addrTest));
    wait(0.2);
    ASSERT_NO_FAILURE(ble.gap().getAddress(&addressType, address));
    printf("%d:%d:%d:%d:%d:%d\n", addrTest[0], addrTest[1], addrTest[2], addrTest[3], addrTest[4], addrTest[5]);
    printf("%d:%d:%d:%d:%d:%d\n", address[0], address[1], address[2], address[3], address[4], address[5]);
    ble.gap().setAddress(Gap::ADDR_TYPE_PUBLIC,temp); 
}
/**
* Test to change advertisement interval
*/
void changeAdvertisingInterval(void)
{
    ASSERT_NO_FAILURE(ble.gap().stopAdvertising());
    ble.gap().setAdvertisingTimeout(0);
    ble.gap().setAdvertisingInterval(500);
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
}

/**
* Test to change advertisement payload
*/
void changeAdvPay(void){
    ASSERT_NO_FAILURE(ble.gap().stopAdvertising());
    ble.gap().clearAdvertisingPayload();
    ble.gap().setAdvertisingTimeout(0);
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::LE_GENERAL_DISCOVERABLE | GapAdvertisingData::BREDR_NOT_SUPPORTED));
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::OUTDOOR_GENERIC));
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayloadTxPower(10));
    uint8_t data[5] = {123,123,123,123,123};
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, data, 5));
    ble.gap().setAdvertisingInterval(500);
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
}

/**
* Test to change add a scan response
*/
void changeScanRes(void){
    ASSERT_NO_FAILURE(ble.gap().stopAdvertising());
    ble.gap().clearAdvertisingPayload();
    ble.gap().clearScanResponse();
    ble.gap().setAdvertisingTimeout(0);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_SCANNABLE_UNDIRECTED);
    uint8_t data2[5] = {50,50,50,50,50};
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, data2, sizeof(data2)));
    uint8_t data[5] = {50,50,50,50,50};
    ASSERT_NO_FAILURE(ble.gap().accumulateScanResponse(GapAdvertisingData::SERVICE_DATA, data, sizeof(data)));
    ble.gap().setAdvertisingInterval(500);
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
}

/**
* Test to change advertisement timeout
*/
void timeoutTest(void){
    ASSERT_NO_FAILURE(ble.gap().stopAdvertising());
    ble.gap().clearAdvertisingPayload();
    ble.gap().clearScanResponse();
    ble.gap().setAdvertisingTimeout(5);
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
}

/**
* Reset function run after every test
*/
void reset(void){
    ble.gap().stopAdvertising();
    ble.gap().clearAdvertisingPayload();
    ble.gap().clearScanResponse();
    ble.gap().setAdvertisingTimeout(0);
    ble.gap().setAdvertisingInterval(1000);
    uint8_t data2[5] = {0,0,0,0,0};
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, data2, sizeof(data2));
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
}

/**
* Controls which tests are run from input from PC
*/
void commandInterpreter(void)
{
    
    unsigned synchroniztion;
    while (true){
        char command[50];
        scanf("%s", command);
        if (!strcmp(command, "setAdvertisingInterval")) changeAdvertisingInterval();
        else if (!strcmp(command, "accumulateAdvertisingPayload")) changeAdvPay();
        else if (!strcmp(command, "setAdvertisingTimeout")) timeoutTest();
        else if (!strcmp(command, "accumulateScanResponse")) changeScanRes();
        else if (!strcmp(command, "iBeaconTest")) setupIBeaconTest();
        else if (!strcmp(command, "setgetAddress")) setAddrTest();
        scanf("%d", &synchroniztion);
        reset();
    }
}

void verifyBasicAssumptions()
{
    ASSERT_NO_FAILURE(ble.init());

    /* TODO: fix. Read in the MAC address of this peripheral. The corresponding central will be
     * commanded to co-ordinate with this address. */

    Gap::Address_t address;
    ASSERT_NO_FAILURE(ble.gap().getAddress(&addressType, address)); /* TODO: if this fails, then bail out with a useful report. */


 //   /*TODO: check that the state is one of the valid values. TODO: if this fails, then bail out with a useful report. */
    Gap::GapState_t state = ble.gap().getState();
    (state.connected == 1 || state.advertising == 1) ? printf("{{failure}} ble.gap().getState() at line %u\r\n", __LINE__ -1) : printf("{{success}}\r\n");

}

int main(void)
{
    verifyBasicAssumptions();

    /* TODO: add a comment to explain the following: should say something about why we
     * need to integrate with host test. */
    printf("{{success}}" "\n" "{{end}}" "\n"); //tells mbedhtrun to finish
    Gap::Address_t address;
    ble.gap().getAddress(&addressType, address);
    unsigned syncer;
    scanf("%d",&syncer); /* scanf to sync with the host PC*/
    printf("%d:%d:%d:%d:%d:%d\n", address[0], address[1], address[2], address[3], address[4], address[5]); /* sends the MAC address to the host PC. */
    commandInterpreter();
}
