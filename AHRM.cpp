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
#include "ble/services/HeartRateService.h"
#include "ble/services/BatteryService.h"
#include "ble/services/DeviceInformationService.h"
#include "LEDService.h"

#define ASSERT_NO_FAILURE(X) (X == BLE_ERROR_NONE) ? (printf("{{success}}\r\n")) : printf("{{failure}} %s at line %u ERROR CODE: %u\r\n", #X, __LINE__, (X));
#define CHECK_EQUALS(X,Y)    ((X)==(Y)) ? (printf("{{sucess}}\r\n")) : printf("{{failure}}\r\n");

BLE  ble;
Gap::Address_t address;

const static char     DEVICE_NAME[]        = "HRMTEST";
static const uint16_t uuid16_list[]        = {GattService::UUID_HEART_RATE_SERVICE,
                                              GattService::UUID_DEVICE_INFORMATION_SERVICE,
                                              LEDService::LED_SERVICE_UUID};


void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason)
{
    ble.gap().startAdvertising(); // restart advertising
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *params){
    printf("Connected to: %d:%d:%d:%d:%d:%d\n",
        params->peerAddr[0], params->peerAddr[1], params->peerAddr[2], params->peerAddr[3], params->peerAddr[4], params->peerAddr[5]);
}

void testDeviceName()
{
    if (ble.gap().getState().connected){
        printf("Device must be disconnected\n");
        return;
    }
    uint8_t deviceName[10];
    uint8_t deviceNameIn[] = {0x4A, 0x4F, 0x53, 0x48, 0x54, 0x45, 0x53, 0x54, 0x00};
    unsigned length = 10;
    ASSERT_NO_FAILURE(ble.gap().setDeviceName(deviceNameIn));
    wait(0.5);
    ASSERT_NO_FAILURE(ble.gap().getDeviceName(deviceName, &length));
    wait(0.5);
    for (int i = 0; i < length; i++){
        printf("%02x ", deviceName[i]);
    }
    printf("\r\n");
    for (int i = 0; i < 8; i++){
        printf("%02x ", deviceNameIn[i]);
    }
    printf("\r\n");
}

void testAppearance()
{
    if ((ble.gap().getState().connected)){
        printf("Device must be disconnected\n");
        return;
    }
    GapAdvertisingData::Appearance appearance;
    ASSERT_NO_FAILURE(ble.gap().setAppearance(GapAdvertisingData::GENERIC_PHONE));
    wait(0.5);
    ASSERT_NO_FAILURE(ble.gap().getAppearance(&appearance));
    wait(0.5);
    printf("%d\r\n", appearance);
}

void connParams()
{
    if ((ble.gap().getState().connected)){
        printf("Device must be disconnected\n");
        return;
    }

    Gap::ConnectionParams_t params;
    Gap::ConnectionParams_t paramsOut = {50,500,0,500};
    Gap::ConnectionParams_t temp;
    ASSERT_NO_FAILURE(ble.gap().getPreferredConnectionParams(&temp));
    ASSERT_NO_FAILURE(ble.gap().setPreferredConnectionParams(&paramsOut));
    ble.gap().getPreferredConnectionParams(&params);
    printf("%d\n", params.minConnectionInterval);
    printf("%d\n", params.maxConnectionInterval);
    printf("%d\n", params.slaveLatency);
    printf("%d\n", params.connectionSupervisionTimeout);
    ble.gap().setPreferredConnectionParams(&temp);

}

void commandInterpreter(void)
{
    const static size_t MAX_SIZEOF_COMMAND = 50;
    while (true) {
        char command[MAX_SIZEOF_COMMAND];
        scanf("%s", command);

        if (!strcmp(command, "setDeviceName"))             testDeviceName();
        else if (!strcmp(command, "setAppearance"))        testAppearance();
        else if (!strcmp(command, "testConnectionParams")) connParams();
    }
}

unsigned verifyBasicAssumptions()
{
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(connectionCallback);

    /* Setup advertising. */
    if(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE)) return 1;
    if(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list))) return 1;
    if(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR)) return 1;
    if(ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME))) return 1;
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms */

    if(ble.gap().startAdvertising()) return 1;
    
    return 0;
}

int main(void)
{
    unsigned errorCode = ble.init();
    uint8_t hrmCounter = 100; // init HRM to 100bps
    HeartRateService hrService(ble, hrmCounter, HeartRateService::LOCATION_FINGER);

    bool initialValueForLEDCharacteristic = false;
    LEDService ledService(ble, initialValueForLEDCharacteristic);

    DeviceInformationService deviceInfo(ble, "ARM", "Model1", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");
    
    errorCode |= verifyBasicAssumptions();
    
    printf("{{success}}\n{{end}}\n"); /* hand over control from the host test to the python script. */
    
    unsigned synchronize;
    scanf("%u", &synchronize);
    
    errorCode ? printf("Initial basic assumptions failed\r\n") : printf("Initial basic assumptions success\r\n");
    
    Gap::AddressType_t addressType;
    ASSERT_NO_FAILURE(ble.gap().getAddress(&addressType, address));
    
    /* write out the MAC address to allow the second level python script to target this device. */
    printf("%d:%d:%d:%d:%d:%d\n", address[0], address[1], address[2], address[3], address[4], address[5]);

    commandInterpreter();
}
