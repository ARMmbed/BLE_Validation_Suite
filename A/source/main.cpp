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

#define ASSERT_NO_FAILURE(CMD) do { \
                    ble_error_t error = (CMD); \
                    if (error == BLE_ERROR_NONE){ \
                        printf("{{success}}\r\n"); \
                    } else{ \
                        printf("{{failure}} %s at line %u ERROR CODE: %u\r\n", #CMD, __LINE__, (error)); \
                        return; \
                    } \
                    }while (0)
                 
#define CHECK_EQUALS(X,Y)    ((X)==(Y)) ? (printf("{{success}}\r\n")) : printf("{{failure}}\r\n");

BLE ble;

typedef void (*funcPtr)();
RawSerial console(USBTX, USBRX);
uint8_t buffer[32];
uint8_t bufferIndex = 0;

void resetStateForNextTest(void);

/**
* Test for advertising using an iBeacon
*/
void setupIBeaconTest(void)
{
    /* setup the ibeacon */
    const static uint8_t uuid[] = {0xE2, 0x0A, 0x39, 0xF4, 0x73, 0xF5, 0x4B, 0xC4,
                                   0xA1, 0x2F, 0x17, 0xD1, 0xAD, 0x07, 0xA9, 0x61};
    uint16_t majorNumber = 1122;
    uint16_t minorNumber = 3344;
    uint16_t txPower     = 0xC8;
    iBeacon ibeacon(ble, uuid, majorNumber, minorNumber, txPower);
    
    uint16_t interval_value = 1000;
    ble.gap().setAdvertisingInterval(interval_value); /* 1000ms. */
    CHECK_EQUALS(ble.gap().getAdvertisingParams().getInterval(), interval_value); 

    ble.gap().setAdvertisingTimeout(0);
    CHECK_EQUALS(ble.gap().getAdvertisingParams().getTimeout(), 0);

    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
    printf("ASSERTIONS DONE\r\n");
}
/**
* Test for setting and getting MAC address
*/
void setAddrTest(void)
{
    Gap::AddressType_t addressType;
    Gap::Address_t origAddress;
    ble.gap().getAddress(&addressType, origAddress);

    const static Gap::Address_t newAddress = {110, 100, 100, 100, 100, 100}; /* A randomly chosen address for assigning to the peripheral. */
    ASSERT_NO_FAILURE(ble.gap().setAddress(Gap::ADDR_TYPE_PUBLIC, newAddress));
    Gap::Address_t fetchedAddress;
    ASSERT_NO_FAILURE(ble.gap().getAddress(&addressType, fetchedAddress));
    printf("ASSERTIONS DONE\r\n");
    printf("%d:%d:%d:%d:%d:%d\n", newAddress[0], newAddress[1], newAddress[2], newAddress[3], newAddress[4], newAddress[5]);
    printf("%d:%d:%d:%d:%d:%d\n", fetchedAddress[0], fetchedAddress[1], fetchedAddress[2], fetchedAddress[3], fetchedAddress[4], fetchedAddress[5]);

    ble.gap().setAddress(Gap::ADDR_TYPE_PUBLIC, origAddress);
}
/**
* Test to change advertisement interval
*/
void changeIntervalTest(void)
{
    ble.gap().setAdvertisingTimeout(0);
    ble.gap().setAdvertisingInterval(500); /* in milliseconds. */
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
    printf("ASSERTIONS DONE\r\n");
}

/**
* Test to change advertisement payload
*/
void changePayloadTest(void)
{

    ble.gap().clearAdvertisingPayload();
    ble.gap().setAdvertisingTimeout(0);

    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::LE_GENERAL_DISCOVERABLE | GapAdvertisingData::BREDR_NOT_SUPPORTED));
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::OUTDOOR_GENERIC));
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayloadTxPower(10)); /* in dbm. */

    const static uint8_t trivialAdvPayload[] = {123, 123, 123, 123, 123};
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, trivialAdvPayload, sizeof(trivialAdvPayload)));

    ble.gap().setAdvertisingInterval(500); /* in milliseconds. */
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
    printf("ASSERTIONS DONE\r\n");
}

/**
* Test to change add a scan response
*/
void responseTest(void)
{
    ble.gap().clearAdvertisingPayload();
    ble.gap().clearScanResponse();
    ble.gap().setAdvertisingTimeout(0);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_SCANNABLE_UNDIRECTED);

    const static uint8_t trivialAdvPayload[] = {50, 50, 50, 50, 50};
    ASSERT_NO_FAILURE(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, trivialAdvPayload, sizeof(trivialAdvPayload)));

    const static uint8_t trivialScanResponse[] = {50, 50, 50, 50, 50};
    ASSERT_NO_FAILURE(ble.gap().accumulateScanResponse(GapAdvertisingData::SERVICE_DATA, trivialScanResponse, sizeof(trivialScanResponse)));

    ble.gap().setAdvertisingInterval(500); /* in  units of milliseconds. */
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
    printf("ASSERTIONS DONE\r\n");
}

/**
* Test to change advertisement timeout.
*/
void setTimeoutTest(void)
{
    ble.gap().clearAdvertisingPayload();
    ble.gap().clearScanResponse();

    ble.gap().setAdvertisingTimeout(5); /* 5 seconds */
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
    printf("ASSERTIONS DONE\r\n");
}

/**
* Reset function run after every test
*/
void resetStateForNextTest(void)
{
    ble.gap().stopAdvertising();
    ble.gap().clearAdvertisingPayload();
    ble.gap().clearScanResponse();
    ble.gap().setAdvertisingTimeout(0);
    ble.gap().setAdvertisingInterval(1000);

    const static uint8_t trivialAdvPayload[] = {0, 0, 0, 0, 0};
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, trivialAdvPayload, sizeof(trivialAdvPayload));
}

void shutdownTest(void)
{
    ASSERT_NO_FAILURE(ble.shutdown());
    ASSERT_NO_FAILURE(ble.init());
    ASSERT_NO_FAILURE(ble.gap().startAdvertising());
    printf("ASSERTIONS DONE\r\n");
}

funcPtr getTest(){

    struct DispatchTableEntry {
        const char * command;
        void (* handler)(void);
    };

    const DispatchTableEntry table[] = {
        {
            "1", resetStateForNextTest
        },
        {
            "setAddr", setAddrTest
        },
        {
            "changeInterval", changeIntervalTest
        },
        {
            "changePayload", changePayloadTest
        },
        {
            "setTimeout", setTimeoutTest
        },
        {
            "response", responseTest
        },
        {
            "shutdown", shutdownTest
        },
        {
            "detect", setupIBeaconTest
        }
    };

    unsigned arraySize = sizeof(table)/sizeof(DispatchTableEntry);
    for (unsigned i = 0; i < arraySize; i++){
        if (!strcmp((const char*)buffer, table[i].command)){
            return table[i].handler;
        }
    }
    return NULL;
}

/**
 * Controls which tests are run from input from PC
 */
void commandInterpreter(void)
{
    funcPtr test = getTest();
    if (test){
        bufferIndex = 0;
        memset(buffer, 0, strlen((char*)buffer));
        test();
    }
}

/**
 * @return 0 if basic assumptions are validated. Non-zero returns are used to
 *     terminate the second-level python script early.
 */
unsigned verifyBasicAssumptions()
{
    if (ble.init()) {
        return 1;
    }

    /* Read in the MAC address of this peripheral. The corresponding central will be
     * commanded to co-ordinate with this address. */
    Gap::AddressType_t addressType;
    Gap::Address_t     address;
    if (ble.gap().getAddress(&addressType, address)) {
        return 1;
    } /* TODO: if this fails, then bail out with a useful report. */

    /* Check that the state is one of the valid values. */
    Gap::GapState_t state = ble.gap().getState();
    if ((state.connected == 1) || (state.advertising == 1)) {
        printf("{{failure}} ble.gap().getState() at line %u\r\n", __LINE__); /* writing out {{failure}} will halt the host test runner. */
        return 1;
    }
    
    const char *version = ble.getVersion();
    printf("%s\r\n", version);

    printf("{{success}}\r\n");
    return 0;
}

void serialHandler(void)
{
    char input = console.getc();
    if (input != '\n' && input != '\r'){
        buffer[bufferIndex++] = input;
    }
    commandInterpreter();
}

void app_start(int, char*[])
{
    unsigned errorCode = verifyBasicAssumptions();

    printf("{{end}}\r\n"); // tells mbedhtrun to finish and hand control over to the second level python script.

    /* Synchronize with the second python script--wait for something to arrive on the console. */
    unsigned syncer;
    scanf("%d",&syncer);

    if (errorCode) {
        printf("Initial basic assumptions failed\r\n"); /* this will halt the second level python script. */
        return;
    }

    /* Refetch the address to write out to the console. */
    Gap::Address_t     address;
    Gap::AddressType_t addressType;
    ASSERT_NO_FAILURE(ble.gap().getAddress(&addressType, address));
    printf("%d:%d:%d:%d:%d:%d\n", address[0], address[1], address[2], address[3], address[4], address[5]); /* sends the MAC address to the host PC. */

    console.attach(serialHandler);
    
    commandInterpreter();
}

#if !defined(YOTTA_MINAR_VERSION_STRING)

int main(void)
{
    app_start(0, NULL);
    return 0;
}

#endif