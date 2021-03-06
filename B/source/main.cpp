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

/* This is the Central side of the beacon test. */

#include "mbed.h"
#include "ble/BLE.h"

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
 * Global static objects.
 */
BLE ble;
Gap::Address_t peerAddress;

void advertisementCallback(const Gap::AdvertisementCallbackParams_t *params)
{
    for (size_t i = 0; i < Gap::ADDR_LEN; i++) {
        if (peerAddress[i] != params->peerAddr[i]) {
            return; /* Filter away adverts all adverts except from the targeted peer. */
        }
    }

    /* emit the last byte of the advert payload. */
    printf("ScanResp: %u, Data: ", params->isScanResponse, *(params->advertisingData + params->advertisingDataLen - 1));
    for (unsigned index = 0; index < params->advertisingDataLen; index++) {
        printf("%u ", params->advertisingData[index]);
    }
    printf("\r\n");
}

void app_start(int, char*[])
{
    printf("{{success}}\r\n{{end}}\r\n"); /* to handover control from the hosttest to the python script. */

    /* Read in the peer address. */
    int x;
    for (unsigned i = 0; i < Gap::ADDR_LEN; i++) {
        scanf("%d", &x);
        peerAddress[i] = (uint8_t)x;
    }

    ASSERT_NO_FAILURE(ble.init());
    ASSERT_NO_FAILURE(ble.gap().setScanParams(500 /* scan interval */, 200 /* scan window */, 0, true /* active scanning */));
    ASSERT_NO_FAILURE(ble.gap().startScan(advertisementCallback));

    printf("ASSERTIONS DONE\r\n");
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
