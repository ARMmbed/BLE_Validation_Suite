/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
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

 /*
 * This test-group presents a command interpreter over the serial console to
 * allow various sub-tests to be executed. It is meant to be driven by a script,
 * but a user can also directly interact with it. It relies upon a useful
 * implementation for RawSerial on the host platform; more specifically, it
 * assumes that it is possible to attach handlers for input characters.
 */

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/services/HeartRateService.h"
#include "ble/services/DeviceInformationService.h"
#include "LEDService.h"
#include "ButtonService.h"

/**
 * Assertion and check macros
 */

/**
 * Execute a command (from BLE_API) and report failure. Note that there is a
 * premature return in the case of a failure.
 *
 * @param[in] CMD
 *                The command (function-call) to be invoked.
 */
#define ASSERT_NO_FAILURE(CMD) do {                                                      \
    ble_error_t error = (CMD);                                                           \
    if (error == BLE_ERROR_NONE) {                                                       \
        printf("{{success}}\r\n");                                                       \
    } else {                                                                             \
        printf("{{failure}} %s at line %u ERROR CODE: %u\r\n", #CMD, __LINE__, (error)); \
        return;                                                                          \
    }                                                                                    \
} while (0)

#define CHECK_EQUALS(X,Y) ((X) == (Y) ? printf("{{success}}\r\n") : printf("{{failure}}\r\n"));

/**
 * Declarations.
 */
typedef void (*CommandHandler_t)(void); /* prototype for a handler of a user command. */

/**
 * Global static objects.
 */
BLE ble;
ButtonService *btnServicePtr;

Gap::Address_t address;

RawSerial console(USBTX, USBRX);
static const size_t SIZEOF_CONSOLE_INPUT_BUFFER = 32; /* should be large enough to capture any command name. */
uint8_t *consoleInputBuffer;
size_t   consoleBufferIndex = 0;

static const char     DEVICE_NAME[] = "HRMTEST";
static const uint16_t uuid16_list[] = {GattService::UUID_HEART_RATE_SERVICE,
                                       GattService::UUID_DEVICE_INFORMATION_SERVICE,
                                       LEDService::LED_SERVICE_UUID};


/**
 * Restarts advertising
 */
void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason)
{
    printf("Disconnected\r\n");
    ble.gap().startAdvertising(); // restart advertising
}

/**
 * When connected prints the bluetooth MAC address of the device connected to
 */
void connectionCallback(const Gap::ConnectionCallbackParams_t *params){
    printf("Connected to: %d:%d:%d:%d:%d:%d\n",
           params->peerAddr[0], params->peerAddr[1], params->peerAddr[2], params->peerAddr[3], params->peerAddr[4], params->peerAddr[5]);
}

/**
 * Tests the set and get Device Name functions
 */
void setDeviceNameTest()
{
    if (ble.gap().getState().connected) {
        printf("Device must be disconnected\n");
        return;
    }

    uint8_t deviceNameIn[] = "Josh-test";
    ASSERT_NO_FAILURE(ble.gap().setDeviceName(deviceNameIn));
    wait(0.5);  /* TODO: remove this. */

    static const size_t MAX_DEVICE_NAME_LEN = 50;
    uint8_t  deviceName[MAX_DEVICE_NAME_LEN];
    unsigned length = MAX_DEVICE_NAME_LEN;
    ASSERT_NO_FAILURE(ble.gap().getDeviceName(deviceName, &length));

    printf("ASSERTIONS DONE\r\n");

    for (unsigned i = 0; i < length; i++) {
        printf("%c", deviceName[i]);
    }
    printf("\r\n");
    for (unsigned i = 0; i < strlen((char *)deviceNameIn); i++) {
        printf("%c", deviceNameIn[i]);
    }
    printf("\r\n");
}

/**
 * Tests the set and get Apeparance functions
 */
void appearanceTest()
{
    if ((ble.gap().getState().connected)) {
        printf("Device must be disconnected\n");
        return;
    }

    ASSERT_NO_FAILURE(ble.gap().setAppearance(GapAdvertisingData::GENERIC_PHONE));
    GapAdvertisingData::Appearance appearance;
    ASSERT_NO_FAILURE(ble.gap().getAppearance(&appearance));
    printf("ASSERTIONS DONE\r\n");

    printf("%d\r\n", appearance);
}

/**
 * Tests the get and set Preferred Connection Params functions
 */
void connParamTest()
{
    if ((ble.gap().getState().connected)) {
        printf("Device must be disconnected\n");
        return;
    }

    Gap::ConnectionParams_t originalParams;
    ASSERT_NO_FAILURE(ble.gap().getPreferredConnectionParams(&originalParams));

    Gap::ConnectionParams_t params;
    Gap::ConnectionParams_t paramsOut = {50, 500, 0, 500};
    ASSERT_NO_FAILURE(ble.gap().setPreferredConnectionParams(&paramsOut));

    printf("ASSERTIONS DONE\r\n");

    ble.gap().getPreferredConnectionParams(&params);

    printf("%d\n", params.minConnectionInterval);
    printf("%d\n", params.maxConnectionInterval);
    printf("%d\n", params.slaveLatency);
    printf("%d\n", params.connectionSupervisionTimeout);

    ble.gap().setPreferredConnectionParams(&originalParams);
}

/**
 * Changes button characteristic to be detected the B device for the notification test
 */
void notificationTest(void) {
    btnServicePtr->updateButtonState(true);
}

/**
 * Returns a pointer to the test function wanting to run. Sets up a table which maps strings to functions.
 */
CommandHandler_t mapInputToHandler(void)
{
    struct DispatchTableEntry {
        const char       *command;
        CommandHandler_t  handler;
    };
    /* list of supported tests. */
    const static DispatchTableEntry table[] = {
        {"setDeviceName", setDeviceNameTest},
        {"appearance",    appearanceTest},
        {"connParam",     connParamTest},
        {"notification",  notificationTest}
    };

    // Checks to see if the inputted string matches an entry in the table
    size_t arraySize = sizeof(table)/sizeof(DispatchTableEntry);
    for (size_t i = 0; i < arraySize; i++) {
        if (!strcmp((const char *)consoleInputBuffer, table[i].command)) {
            return table[i].handler;
        }
    }

    return NULL;
}

/**
 * If there is a match test name in the consoleInputBuffer, this will run the test; otherwise return immediately.
 */
void commandInterpreter(void)
{
    CommandHandler_t test = mapInputToHandler();
    if (test) {
        /* If we've found a test to run, we can ignore the rest of the consoleInputBuffer. */
        consoleBufferIndex = 0;
        memset(consoleInputBuffer, 0, SIZEOF_CONSOLE_INPUT_BUFFER);

        test(); /* dispatch the test. */
    }
}

/**
 * handler for the serial interrupt, ignores \r and \n characters
 */
void serialHandler(void)
{
    char input = console.getc();
    if ((input != '\n') && (input != '\r')) {
        consoleInputBuffer[consoleBufferIndex++] = input;
    } else {
        commandInterpreter();
    }
}

/**
 * @return 0 if basic assumptions are validated. Non-zero returns are used to
 *     terminate the second-level python script early.
 */
unsigned verifyBasicAssumptions()
{
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(connectionCallback);

    /* Setup advertising. */
    if (ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE)) {
        return 1;
    }
    if (ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list))) {
        return 1;
    }
    if (ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR)) {
        return 1;
    }
    if (ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME))) {
        return 1;
    }
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms */

    if (ble.gap().startAdvertising()) {
        return 1;
    }

    const char *version = ble.getVersion();
    printf("%s\r\n", version);
    if (!strcmp(version, "")) return 1;
    return 0;
}

void app_start(int, char*[])
{
    unsigned errorCode = ble.init();
    if (errorCode == 0) {
        uint8_t                   hrmCounter = 100; // init HRM to 100bps
        HeartRateService         *hrService  = new HeartRateService(ble, hrmCounter, HeartRateService::LOCATION_FINGER);

        bool                      initialValueForLEDCharacteristic = false;
        LEDService               *ledService                       = new LEDService(ble, initialValueForLEDCharacteristic);

        btnServicePtr = new ButtonService(ble, false);
    }
    errorCode |= verifyBasicAssumptions();

    if (errorCode == 0) {
        printf("{{success}}\r\n{{end}}\r\n"); /* hand over control from the host test to the python script. */
    } else {
        printf("{{failure}}\r\n{{end}}\r\n"); /* hand over control from the host test to the python script. */
    }

    unsigned synchronize;
    scanf("%u", &synchronize);

    if (errorCode != 0) {
        printf("Initial basic assumptions failed\r\n");
        return;
    }

    Gap::AddressType_t addressType;
    ASSERT_NO_FAILURE(ble.gap().getAddress(&addressType, address));

    /* write out the MAC address to allow the second level python script to target this device. */
    printf("%d:%d:%d:%d:%d:%d\n", address[0], address[1], address[2], address[3], address[4], address[5]);

    consoleInputBuffer = (uint8_t*)malloc(SIZEOF_CONSOLE_INPUT_BUFFER);
    memset(consoleInputBuffer, 0, SIZEOF_CONSOLE_INPUT_BUFFER);

    console.attach(serialHandler);
}

/**
 * main() is needed only for mbed-classic. mbed OS triggers app_start() automatically.
 */
#ifndef YOTTA_CFG_MBED_OS
int main(void)
{
    app_start(0, NULL);
    return 0;
}
#endif
