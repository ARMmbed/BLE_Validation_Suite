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
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"

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
Gap::Address_t address;

RawSerial console(USBTX, USBRX);
static const size_t SIZEOF_CONSOLE_INPUT_BUFFER = 32; /* should be large enough to capture any command name. */
uint8_t *consoleInputBuffer;
size_t  consoleBufferIndex = 0;

DiscoveredCharacteristic* HRMCharacteristic;
DiscoveredCharacteristic* LEDCharacteristic;
DiscoveredCharacteristic* BTNCharacteristic;
bool HRMFound = false;
bool LEDFound = false;
bool BTNFound = false;

Gap::Handle_t deviceAHandle;

void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason)
{
    printf("Disconnected\r\n");
}

/*
 * Call back when a service is discovered
 */
void serviceDiscoveryCallback(const DiscoveredService *service)
{
    if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
        printf("S UUID-%x attrs[%u %u]\r\n", service->getUUID().getShortUUID(), service->getStartHandle(), service->getEndHandle());
    } else {
        printf("S UUID-");
        const uint8_t *longUUIDBytes = service->getUUID().getBaseUUID();
        for (unsigned i = 0; i < UUID::LENGTH_OF_LONG_UUID; i++) {
            printf("%02x", longUUIDBytes[i]);
        }
        printf("attrs[%u %u]\r\n", service->getStartHandle(), service->getEndHandle());
    }
}

/*
 * Call back when a characteristic is discovered
 */
void characteristicDiscoveryCallback(const DiscoveredCharacteristic *characteristicP)
{
    if (characteristicP->getUUID().getShortUUID() == 0x2a37) { /* Searches for HRM Characteristic*/
        HRMCharacteristic = new DiscoveredCharacteristic(*characteristicP);
        HRMFound          = true;
    }
    if (characteristicP->getUUID().getShortUUID() == 0xA001) { /* Searches for LED Characteristic*/
        LEDCharacteristic = new DiscoveredCharacteristic(*characteristicP);
        LEDFound          = true;
    }
    if (characteristicP->getUUID().getShortUUID() == 0xA003) {
        BTNCharacteristic = new DiscoveredCharacteristic(*characteristicP);
        BTNFound          = true;
    }
}

/*
 * Call back when device is connected
 */
void connectionCallback(const Gap::ConnectionCallbackParams_t *params)
{
    printf("Connected to: %d:%d:%d:%d:%d:%d\n",
           params->peerAddr[0], params->peerAddr[1], params->peerAddr[2], params->peerAddr[3], params->peerAddr[4], params->peerAddr[5]);
    if (params->role == Gap::CENTRAL) {
        deviceAHandle = params->handle; /* Handle for device A so it is it possible to disconnect*/
        ASSERT_NO_FAILURE(ble.gattClient().launchServiceDiscovery(params->handle, serviceDiscoveryCallback, characteristicDiscoveryCallback));
    }
}

/*
 * The callback for reading a characteristic, print depends on what characteristic is read
 */
void readCharacteristic(const GattReadCallbackParams *response)
{
    if (response->handle == HRMCharacteristic->getValueHandle()) {
        printf("HRMCounter: %d\n",  response->data[1]);
    }
    if (response->handle == LEDCharacteristic->getValueHandle()) {
        printf("LED: %d\n", response->data[0]);
    }
    if (response->handle == BTNCharacteristic->getValueHandle()) {
        printf("BTN: %d\n", response->data[0]);
    }
}

/*
 * Tests connecting devices. Devices must be disconnected for this test
 */
void connectTest()
{
    if (!(ble.gap().getState().connected)) {
        ASSERT_NO_FAILURE(ble.gap().connect(address, Gap::ADDR_TYPE_RANDOM_STATIC, NULL, NULL));
    } else {
        printf("Devices already connected\n");
    }
}

/*
 * Tests reading from to the heart rate characteristic. Devices need to be connected for this test.
 */
void readTest(){
    if (!(ble.gap().getState().connected)) {
        printf("Devices must be connected before this test can be run\n");
        return;
    }
    if (HRMFound) {
        ASSERT_NO_FAILURE(HRMCharacteristic->read());
    } else {
        printf("Characteristic not found\r\n");
    }
}

/**
 * Tests writing to the LED Characteristic. Then reads from the callback to verify that the write is correct.
 * Devices need to be connected for this test.
 */
void writeTest()
{
    if (!(ble.gap().getState().connected)) {
        printf("Devices must be connected before this test can be run\n");
        return;
    }
    if (LEDFound) {
        uint8_t write_value = 1;
        ASSERT_NO_FAILURE(LEDCharacteristic->write(sizeof(write_value), &write_value)); /* When write finishes, writeCallback is called */
    } else {
        printf("Characeristic not found\r\n");
    }
}

/**
 * Tests disconnecting devices. If it is already connected it prints a message
 */
void disconnectTest()
{
    if ((ble.gap().getState().connected)) {
        ASSERT_NO_FAILURE(ble.gap().disconnect(deviceAHandle, Gap::REMOTE_USER_TERMINATED_CONNECTION));
    } else {
        printf("Devices not connected\n");
    }
}

void notificationTest()
{
    if (!ble.gap().getState().connected) {
        printf("Devices must be connected before this test can be run\n");
        return;
    }
    if (BTNFound) {
        uint16_t value = BLE_HVX_NOTIFICATION;
        ASSERT_NO_FAILURE(ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ,
                                   deviceAHandle,
                                   BTNCharacteristic->getValueHandle() + 1, /* HACK Alert. We're assuming that CCCD descriptor immediately follows the value attribute. */
                                   sizeof(uint16_t),                        /* HACK Alert! size should be made into a BLE_API constant. */
                                   reinterpret_cast<const uint8_t *>(&value)));
    } else {
        printf("Characteristic not found\r\r");
    }
}

/**
 * Call back for writing to LED characteristic.
 */
void writeCallback(const GattWriteCallbackParams *params)
{
    if (params->handle == LEDCharacteristic->getValueHandle()) {
        ASSERT_NO_FAILURE(LEDCharacteristic->read());
    } else if (params->handle == BTNCharacteristic->getValueHandle() + 1) {
        printf("Sync\r\n");
    }
}

void hvxCallback(const GattHVXCallbackParams *params) {
    printf("Button: ");
    for (unsigned index = 0; index < params->len; index++) {
        printf("%02x", params->data[index]);
    }
    printf("\r\n");
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
    const DispatchTableEntry table[] = {
        {"connect",      connectTest},
        {"disconnect",   disconnectTest},
        {"read",         readTest},
        {"write",        writeTest},
        {"notification", notificationTest}
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

void app_start(int, char*[])
{
    printf("{{end}}\n"); /* Hands control over to Python script */

    unsigned x;
    for (unsigned i = 0; i < Gap::ADDR_LEN; i++) {
        scanf("%d", &x);
        address[i] = (uint8_t)x;
    }

    ASSERT_NO_FAILURE(ble.init());
    ASSERT_NO_FAILURE(ble.gap().setScanParams(500 /* scan interval */, 200 /* scan window */));
    printf("ASSERTIONS DONE\r\n");

    ble.gap().onConnection(connectionCallback);
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gattClient().onDataRead(readCharacteristic);
    ble.gattClient().onDataWrite(writeCallback);
    ble.gattClient().onHVX(hvxCallback);

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
