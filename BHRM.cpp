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
#include "BLE.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"
#define DUMP_ADV_DATA 0

#define ASSERT_NO_FAILURE(X)  (X == BLE_ERROR_NONE) ? (printf("{{success}}\r\n")) : printf("{{failure}} %s at line %u ERROR CODE: %u\r\n", #X, __LINE__, (X));
#define CHECK_EQUALS(X,Y) ((X)==(Y)) ? (printf("{{sucess}}\n")) : printf("{{failure}}\n");

BLE                 ble;
Gap::Address_t      address;

DiscoveredCharacteristic HRMCharacteristic;
bool HRMFound =          false;
DiscoveredCharacteristic LEDCharacteristic;
bool LEDFound =          false;
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
    if (characteristicP->getUUID().getShortUUID() == 0x2a37) { 
        HRMCharacteristic = *characteristicP;
        HRMFound          = true;
    }
    if (characteristicP->getUUID().getShortUUID() == 0xA001){
        LEDCharacteristic = *characteristicP;
        LEDFound          = true;   
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
        deviceAHandle = params->handle;
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
}

/*
 * Tests connecting devices. Devices must be disconnected for this test
 */
void connectTest()
{
    if (!(ble.gap().getState().connected)){
        ASSERT_NO_FAILURE(ble.gap().connect(address,Gap::ADDR_TYPE_RANDOM_STATIC,NULL,NULL));
    }
    else printf("Devices already connected\n");
}

/*
 * Tests reading from to the heart rate characteristic. Devices need to be connected for this test.
 */
void readTest(){
    if (!(ble.gap().getState().connected)){
        printf("Devices must be connected before this test can be run\n");
        return;
    }
    if (HRMFound){
        ASSERT_NO_FAILURE(HRMCharacteristic.read());    
    } else{
        printf("Characteristic not found\r\n");
    }
}

/**
 * Tests writing to the LED Characteristic. Then reads from the callback to verify that the write is correct.
 * Devices need to be connected for this test.
 */
void writeTest()
{
    if (!(ble.gap().getState().connected)){
        printf("Devices must be connected before this test can be run\n");
        return;
    }
    if (LEDFound){
        uint8_t write_value = 1;
        ASSERT_NO_FAILURE(LEDCharacteristic.write(sizeof(write_value),&write_value));
    } else{
        printf("Characeristic not found\r\n");
    }
}

/** 
 * Tests disconnecting devices. If it is already connected it prints a message
 */
void disconnectTest()
{
    if ((ble.gap().getState().connected)){
        ASSERT_NO_FAILURE(ble.gap().disconnect(deviceAHandle, Gap::REMOTE_USER_TERMINATED_CONNECTION));
    }
    else printf("Devices not connected\n");        
}

/**
 * Controls which tests are run from input from PC
 */
void commandInterpreter()
{
    char command[50];
    while(true){
        scanf("%s", command);
        if (!strcmp(command, "connect")) connectTest();
        else if (!strcmp(command, "disconnect")) disconnectTest();
        else if (!strcmp(command, "read")) readTest();
        else if (!strcmp(command, "write")) writeTest();
    }    
}

/**
 * Call back for writing to LED characteristic. 
 */
void writeCallback(const GattWriteCallbackParams *params){
    ASSERT_NO_FAILURE(LEDCharacteristic.read());
}

int main(void)
{
    printf("{{end}}\n");
    scanf("%hhu",&address[0]);
    scanf("%hhu",&address[1]);
    scanf("%hhu",&address[2]);
    scanf("%hhu",&address[3]);
    scanf("%hhu",&address[4]);
    scanf("%hhu",&address[5]);
    
    ASSERT_NO_FAILURE(ble.init());
    ASSERT_NO_FAILURE(ble.gap().setScanParams(500 /* scan interval */, 200 /* scan window */));
    
    ble.gap().onConnection(connectionCallback);
    ble.gattClient().onDataRead(readCharacteristic);
    ble.gattClient().onDataWrite(writeCallback);
    commandInterpreter();
}