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

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"

#define ASSERT_NO_FAILURE(CMD) do { \
                    ble_error_t error = (CMD); \
                    if (error == BLE_ERROR_NONE){ \
                        printf("{{success}}\r\n"); \
                    } else{ \
                        printf("{{failure}} %s at line %u ERROR CODE: %u\r\n", #CMD, __LINE__, (error)); \
                        return; \
                    } \
                    }while (0)
#define ASSERT_NO_FAILURE_INT(CMD) do { \
                    ble_error_t error = (CMD); \
                    if (error == BLE_ERROR_NONE){ \
                        printf("{{success}}\r\n"); \
                    } else{ \
                        printf("{{failure}} %s at line %u ERROR CODE: %u\r\n", #CMD, __LINE__, (error)); \
                        return 1; \
                    } \
                    }while (0)              
#define CHECK_EQUALS(X,Y)    ((X)==(Y)) ? (printf("{{success}}\r\n")) : printf("{{failure}}\r\n");

BLE                      ble;
Gap::Address_t           address;

DiscoveredCharacteristic HRMCharacteristic;
bool                     HRMFound =          false;
DiscoveredCharacteristic LEDCharacteristic;
bool                     LEDFound =          false;
// DiscoveredCharacteristic BTNCharacteristic;
// bool                     BTNFound =          false;
Gap::Handle_t            deviceAHandle;

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
        HRMCharacteristic = *characteristicP;
        HRMFound          = true;
    }
    if (characteristicP->getUUID().getShortUUID() == 0xA001) { /* Searches for LED Characteristic*/
        LEDCharacteristic = *characteristicP;
        LEDFound          = true;
    }
    // if (characteristicP->getUUID().getShortUUID() == 0xA003) {
    //     BTNCharacteristic = *characteristicP;
    //     BTNFound          = true;    
    // }
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
    if (response->handle == HRMCharacteristic.getValueHandle()) {
        printf("HRMCounter: %d\n",  response->data[1]);
    }
    if (response->handle == LEDCharacteristic.getValueHandle()) {
        printf("LED: %d\n", response->data[0]);
    }
    // if (response->handle == BTNCharacteristic.getValueHandle()) {
    //     printf("BTN: %d\n", response->data[0]);    
    // }
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
        ASSERT_NO_FAILURE(HRMCharacteristic.read());
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
        ASSERT_NO_FAILURE(LEDCharacteristic.write(sizeof(write_value), &write_value)); /* When write finishes, writeCallback is called */
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

// void notificationTest()
// {
//     if (!ble.gap().getState().connected) {
//         printf("Devices must be connected before this test can be run\n");
//         return;
//     }
//     if (BTNFound) {
//         uint16_t value = BLE_HVX_NOTIFICATION;
//         ASSERT_NO_FAILURE(ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ,
//                                    deviceAHandle,
//                                    BTNCharacteristic.getValueHandle() + 1, /* HACK Alert. We're assuming that CCCD descriptor immediately follows the value attribute. */
//                                    sizeof(uint16_t),                          /* HACK Alert! size should be made into a BLE_API constant. */
//                                    reinterpret_cast<const uint8_t *>(&value)));    
//     } else {
//         printf("Characteristic not found\r\r");    
//     }
// }

/**
 * Controls which tests are run from input from PC
 */
void commandInterpreter()
{
    char command[50];
    while (true) {
        scanf("%s", command); /* Takes a string from the host test and decides what test to use. */
        if (!strcmp(command, "connect")) {
            connectTest();
        } else if (!strcmp(command, "disconnect")) {
            disconnectTest();
        } else if (!strcmp(command, "read")) {
            readTest();
        } else if (!strcmp(command, "write")) {
            writeTest();
        // } else if (!strcmp(command, "notification")) {
        //     notificationTest();
        }
    }
}

/**
 * Call back for writing to LED characteristic.
 */
void writeCallback(const GattWriteCallbackParams *params)
{
    if (params->handle == LEDCharacteristic.getValueHandle()) {
        ASSERT_NO_FAILURE(LEDCharacteristic.read());   
    // } else if (params->handle == BTNCharacteristic.getValueHandle() + 1) {
    //     printf("Sync\r\n");   
    }
}

void hvxCallback(const GattHVXCallbackParams *params) {
    printf("Button: ");
    for (unsigned index = 0; index < params->len; index++) {
        printf("%02x", params->data[index]);
    }
    printf("\r\n");
}

void app_start(int, char* [])
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
    ble.gattClient().onDataRead(readCharacteristic);
    ble.gattClient().onDataWrite(writeCallback);
    ble.gattClient().onHVX(hvxCallback);
    commandInterpreter();
}