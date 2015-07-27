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

#define BLE_CHECK(X)  (X == BLE_ERROR_NONE) ? (printf("{{success}}\r\n")) : printf("{{failure}} %s at line %u ERROR CODE: %u\r\n", #X, __LINE__, (X));
#define BLE_EQUAL(X,Y) ((X)==(Y)) ? (printf("{{sucess}}\n")) : printf("{{failure}}\n");

BLE                 ble;
Gap::Address_t      address;
Gap::AddressType_t *addressType;

DiscoveredCharacteristic HRMCharacteristic;
bool HRMFound =          false;
DiscoveredCharacteristic LEDCharacteristic;
bool LEDFound =          false;
Gap::Handle_t            deviceAHandle;

void advertisementCallback(const Gap::AdvertisementCallbackParams_t *params) {
    for (int i = 0; i < 5; i++){
        if(address[i] != params->peerAddr[i]){
            return;    
        }
    }
}

void serviceDiscoveryCallback(const DiscoveredService *service) {
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

void characteristicDiscoveryCallback(const DiscoveredCharacteristic *characteristicP) {
    if (characteristicP->getShortUUID() == 0x2a37) { 
        HRMCharacteristic        = *characteristicP;
        HRMFound = true;
    }
    if (characteristicP->getShortUUID() == 0xA001){
        LEDCharacteristic = *characteristicP;
        LEDFound = true;   
    }
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *params){
    printf("Connected to: %d:%d:%d:%d:%d:%d\n", params->peerAddr[0], params->peerAddr[1], params->peerAddr[2], params->peerAddr[3], params->peerAddr[4], params->peerAddr[5]); 
    if (params->role == Gap::CENTRAL) {
//        ble.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallback);
        deviceAHandle = params->handle;
        BLE_CHECK(ble.gattClient().launchServiceDiscovery(params->handle, serviceDiscoveryCallback, characteristicDiscoveryCallback));
        //BLE_CHECK(ble.gattClient().launchServiceDiscovery(params->handle, serviceDiscoveryCallback, characteristicDiscoveryCallback, 0x180d, 0x2a37));
        //BLE_CHECK(ble.gattClient().launchServiceDiscovery(params->handle, serviceDiscoveryCallback, characteristicDiscoveryCallback, 0xA000, 0xA001));
    }
}

void triggerToggledWrite(const GattReadCallbackParams *response) {
    if (response->handle == HRMCharacteristic.getValueHandle()) {
        printf("HRMCounter: %d\n",  response->data[1]);
    }
    if (response->handle == LEDCharacteristic.getValueHandle()) {
        printf("LED: %d\n", response->data[0]);
        
    }
}

void connectTest(){
    if (!(ble.gap().getState().connected)){
        BLE_CHECK(ble.gap().connect(address,Gap::ADDR_TYPE_RANDOM_STATIC,NULL,NULL));
    }
    else printf("Devices already connected\n");
}

void readTest(){
    myled = 0;
    if (!(ble.gap().getState().connected)){
        printf("Devices must be connected before this test can be run\n");
        return;
    }
    if (HRMFound){
        BLE_CHECK(HRMCharacteristic.read());    
    }
}
void writeTest(){
    if (!(ble.gap().getState().connected)){
        printf("Devices must be connected before this test can be run\n");
        return;
    }
    uint8_t write_value = 1;
    if (LEDFound){
        BLE_CHECK(LEDCharacteristic.write(sizeof(write_value),&write_value));
        wait(0.5);
        BLE_CHECK(LEDCharacteristic.read());
    }
}

void disconnectTest(){
    if ((ble.gap().getState().connected)){
        BLE_CHECK(ble.gap().disconnect(deviceAHandle, Gap::REMOTE_USER_TERMINATED_CONNECTION));
    }
    else printf("Devices not connected\n");        
}

void commandInterpreter(){
    char command[50];
    while(true){
        scanf("%s", command);
        if (!strcmp(command, "connect")) connectTest();
        else if (!strcmp(command, "disconnect")) disconnectTest();
        else if (!strcmp(command, "read")) readTest();
        else if (!strcmp(command, "write")) writeTest();
    }    
}

int main(void)
{
    myled = 1;
    printf("{{success}}" "\n" "{{end}}" "\n");
    ble.init();
    scanf("%hhu",&address[0]);
    scanf("%hhu",&address[1]);
    scanf("%hhu",&address[2]);
    scanf("%hhu",&address[3]);
    scanf("%hhu",&address[4]);
    scanf("%hhu",&address[5]);
    
    
    BLE_CHECK(ble.gap().setScanParams(500 /* scan interval */, 200 /* scan window */));
    BLE_CHECK(ble.gap().startScan(advertisementCallback));
    ble.gap().onConnection(connectionCallback);
    ble.gattClient().onDataRead(triggerToggledWrite);
    
    commandInterpreter();
}