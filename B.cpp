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
#define DUMP_ADV_DATA 0

#define ASSERT_NO_FAILURE(X)  (X == BLE_ERROR_NONE) ? (printf("{{success}}\r\n")) : printf("{{failure}} %s at line %u\r\n", #X, __LINE__);
#define CHECK_EQUALS(X,Y) ((X)==(Y)) ? (printf("{{sucess}}\n")) : printf("{{failure}}\n");

BLE        ble;
uint8_t address[6];



void advertisementCallback(const Gap::AdvertisementCallbackParams_t *params) {
    for (int i = 0; i < 5; i++){
        if(address[i] != params->peerAddr[i]){
            return;    
        }
    }
    printf("ScanResp: %u, Data: %d\r\n", params->isScanResponse, *(params->advertisingData + params->advertisingDataLen -1));
//    printf("Adv peerAddr: [%02x %02x %02x %02x %02x %02x] rssi %d, ScanResp: %u, AdvType: %u, Data: %d\r\n",
//           params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
//           params->rssi, params->isScanResponse, params->type, *(params->advertisingData + params->advertisingDataLen -1));
           
//    printf("Adv peerAddr: [%d %d %d %d %d %d] rssi %d, ScanResp: %u, AdvType: %u\r\n",
//           params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
//          params->rssi, params->isScanResponse, params->type);
           
#if DUMP_ADV_DATA
    for (unsigned index = 0; index < params->advertisingDataLen; index++) {
        printf("%02x ", params->advertisingData[index]);
    }
    printf("\r\n");
#endif /* DUMP_ADV_DATA */
}


int main(void)
{
    printf("{{success}}" "\n" "{{end}}" "\n");
    for (int i = 0; i < 6; i++){
        scanf("%hhu",&address[i]);    
    }
    
    ASSERT_NO_FAILURE(ble.init());

    ASSERT_NO_FAILURE(ble.gap().setScanParams(500 /* scan interval */, 200 /* scan window */, 0, true));
    ASSERT_NO_FAILURE(ble.gap().startScan(advertisementCallback));

 
}