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

#include "ble-blocktransfer/BlockTransferClient.h"
#include "ble-blocktransfer/BlockTransferService.h"

#include "mbed-block/BlockStatic.h"
#include "mbed-block/BlockCollection.h"

/*****************************************************************************/
/* Configuration                                                             */
/*****************************************************************************/

#define VERBOSE_DEBUG_OUTPUT 1
#define GAP_CENTRAL_IS_GATT_CLIENT 1

const uint32_t maxWriteLength = 64;
const char DEVICE_NAME[] = "BlockTransfer";
const UUID uuid(0xAAAA);

RawSerial console(USBTX, USBRX);
uint8_t *buffer;
uint8_t bufferIndex = 0;

#if defined(MICROBIT)
#define BUTTON1 BUTTON_A
#define BUTTON2 BUTTON_B
maxWriteLength = 256;
#endif

/*****************************************************************************/
/* Variables                                                                 */
/*****************************************************************************/

// BLE_API ble device
static BLE ble;

// Transfer large blocks of data on platforms without Fragmentation-And-Recombination
static BlockTransferService bts;
static BlockTransferClient btc;

// debug led - blinks to show liveness
static Ticker ticker;
static DigitalOut mbed_led1(LED1);

// measure throughput
static Timer watch;

// buffers for sending and receiving data
static uint8_t receiveBuffer[maxWriteLength];
BlockStatic receiveBlock(receiveBuffer, sizeof(receiveBuffer));

/*  Combine 4 separate Block objects into one using the BlockCollection class.

    This can be used for combining data in flash with headers and footers
    stored in RAM into a single Block object which can be send using the
    Block Transfer Service.
*/
#if 1
static uint8_t writeBuffer[maxWriteLength];
BlockStatic writeBlock(writeBuffer, sizeof(writeBuffer));
#else
static uint8_t writeBuffer1[maxWriteLength / 4];
static uint8_t writeBuffer2[maxWriteLength / 4];
static uint8_t writeBuffer4[maxWriteLength / 4];

BlockStatic writeBlockFragment1(writeBuffer1, sizeof(writeBuffer1));
BlockStatic writeBlockFragment2(writeBuffer2, sizeof(writeBuffer2));
BlockStatic writeBlockFragment4(writeBuffer4, sizeof(writeBuffer4));

// this block is stored in non-volatile memory
static const uint8_t writeBuffer3[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
        0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
        0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
        0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
        0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
        0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
        0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
        0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

BlockStaticReadOnly writeBlockFragment3(writeBuffer3, sizeof(writeBuffer3));
BlockCollection writeBlock(&writeBlockFragment1);
#endif

// enable buttons to initiate transfer
// static InterruptIn button1(BUTTON1);
// static InterruptIn button2(BUTTON2);

// BLE book keeping
static bool scanning = false;
static Gap::Handle_t connectionHandle;
static bool peripheralIsServer = true;

// send notification when data is ready to be read
static bool sendHello = false;
static void signalReady();

/*****************************************************************************/
/* Debug                                                                     */
/*****************************************************************************/
/*
    Called once every second. Blinks LED.
*/
void periodicCallback(void)
{
    mbed_led1 = !mbed_led1;
}

/*****************************************************************************/
/* Block Transfer Server                                                     */
/*****************************************************************************/

/*
    Function called when device receives a read request over BLE.
*/
Block* blockServerReadHandler(uint32_t offset)
{
    printf("main: block read: %4lu %6lu\r\n", receiveBlock.getLength(), offset);

#if VERBOSE_DEBUG_OUTPUT
    for (std::size_t idx = 0; idx < receiveBlock.getLength(); idx++)
    {
        printf("%02X", receiveBlock.at(idx));
    }
    printf("\r\n");
#endif

    return &receiveBlock;
}

/*
    Function called when data has been written over BLE.
*/
Block* blockServerWriteHandler(Block* block)
{
    printf("main: block write: length: %4lu offset: %6lu time: %3d ms\r\n", block->getLength(), block->getOffset(), watch.read_ms());

#if VERBOSE_DEBUG_OUTPUT
    for (std::size_t idx = 0; idx < block->getLength(); idx++)
    {
        printf("%02X", block->at(idx));
    }
    printf("\r\n");
#endif

    // the Block Transfer service is not busy
    if (bts.ready())
    {
        // signal data is available to be read
        signalReady();
    }

    // reset timer to measure time to next block
    watch.reset();

    /*
        Return block to Block Transfer Service.
        This can be a different block than the one just received.
    */
    return block;
}


/*****************************************************************************/
/* Block Transfer Client                                                     */
/*****************************************************************************/

/*
    Handler called when Block Transfer Client Write is finished.
*/
void clientWriteDone()
{
    printf("main: write done\r\n");
}

/*
    Handler called when Block Transfer Client Read completes.
*/
void clientReadDone(Block* block)
{
    printf("main: read done\r\n");

#if VERBOSE_DEBUG_OUTPUT
    // print block content
    for (std::size_t idx = 0; idx < block->getLength(); idx++)
    {
        printf("%02X", block->at(idx));
    }
    printf("\r\n");
#endif

    // compare read and write block
    bool matches = true;

    if (block->getLength() != writeBlock.getLength())
    {
        matches = false;
    }
    else
    {
        for (std::size_t idx = 0; idx < block->getLength(); idx++)
        {
            matches = matches && (block->at(idx) == writeBlock.at(idx));
        }
    }

    if (matches)
    {
        printf("main: buffers match\r\n");
    }
    else
    {
        printf("main: buffers differ\r\n");
    }

    // disconnect
    ble.gap().disconnect(connectionHandle, Gap::REMOTE_USER_TERMINATED_CONNECTION);
}

/*
    Block Transfer Client received notification.

    Write to server is complete, and server signals that there is data to read.
*/
void clientNotification()
{
    printf("main: client notification\r\n");

    btc.read(&receiveBlock, clientReadDone);
}

/*
    Block Transfer Client has been initialized properbly when this handler is called.
*/
void clientReady()
{
    printf("main: client ready\r\n");

    // populate write buffer with data
    writeBlock.setLength(maxWriteLength);

    for (std::size_t idx = 0; idx < writeBlock.getLength(); idx++)
    {
        writeBlock.at(idx) = idx;
    }

#if VERBOSE_DEBUG_OUTPUT
    for (std::size_t idx = 0; idx < writeBlock.getLength(); idx++)
    {
        printf("%02X", writeBlock.at(idx));
    }
    printf("\r\n");
#endif

    // start transfer
    btc.write(&writeBlock, clientWriteDone);
}

/*****************************************************************************/
/* BLE Central                                                               */
/*****************************************************************************/

/*
    Handler for when advertisement beacons are received.
*/
void advertisementCallback(const Gap::AdvertisementCallbackParams_t* params)
{
#if VERBOSE_DEBUG_OUTPUT
    printf("adv peerAddr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, AdvertisementType %u\r\n",
           params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
           params->rssi, params->isScanResponse, params->type);
#endif

    // scan through advertisement data
    for (uint8_t idx = 0; idx < params->advertisingDataLen; )
    {
        uint8_t fieldLength = params->advertisingData[idx];
        uint8_t fieldType = params->advertisingData[idx + 1];
        const uint8_t* fieldValue = &(params->advertisingData[idx + 2]);

        idx += fieldLength + 1;

        // find 16-bit service IDs
        if ((fieldType == GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS) ||
            (fieldType == GapAdvertisingData::INCOMPLETE_LIST_16BIT_SERVICE_IDS))
        {
            // compare short UUID
            UUID beaconUUID((fieldValue[1] << 8) | fieldValue[0]);

            if (beaconUUID == uuid)
            {
                // set lowest latency for fastest transfer
                Gap::ConnectionParams_t fast;
                ble.gap().getPreferredConnectionParams(&fast);
                fast.minConnectionInterval = 16; // 20 ms
                fast.maxConnectionInterval = 32; // 40 ms
                fast.slaveLatency = 0;

                // connect to peripheral, this stops the scanning
                ble.gap().connect(params->peerAddr, Gap::ADDR_TYPE_RANDOM_STATIC, &fast, NULL);
                break;
            }
        }
    }
}

/*
    Functions called when BLE device has been connected.
*/
void whenConnected(const Gap::ConnectionCallbackParams_t* params)
{
    printf("main: Connected: %d %d %d\r\n", params->connectionParams->minConnectionInterval,
                                            params->connectionParams->maxConnectionInterval,
                                            params->connectionParams->slaveLatency);

    // save connection handle
    connectionHandle = params->handle;

    if (params->role == Gap::PERIPHERAL)
    {
        printf("main: peripheral\r\n");

        // Instantiate a BlockTransferClient object if the peripheral is not in server mode
        if (peripheralIsServer == false)
        {
            // initialize Block Transfer client
            btc.init(clientReady, uuid, params->handle);
            btc.onNotification(clientNotification);
        }
        else
        {
            watch.start();
        }
    }
    else
    {
        printf("main: central\r\n");
        scanning = false;

        // Instantiate a BlockTransferClient object if the peripheral is in server mode
        if (peripheralIsServer == true)
        {
            // initialize Block Transfer client
            btc.init(clientReady, uuid, params->handle);
            btc.onNotification(clientNotification);
        }
        else
        {
            watch.start();
        }
    }
}

/*
    Function called when BLE device has been disconnected.
*/
void whenDisconnected(Gap::Handle_t, Gap::DisconnectionReason_t)
{
    printf("main: Disconnected!\r\n");
    printf("main: Restarting the advertising process\r\n");

    watch.stop();
    watch.reset();

    ble.gap().startAdvertising();
}

/*****************************************************************************/
/* Buttons                                                                   */
/*****************************************************************************/

/*
    Start or stop scanning. Device becomes central.
*/
void button1ISR()
{
    if (!scanning)
    {
        scanning = true;
        printf("start scan\r\n");
        ble.gap().setScanParams(1000, 1000);
        ble.gap().startScan(advertisementCallback);
    }
    else
    {
        scanning = false;
        printf("stop scan\r\n");
        ble.gap().stopScan();
    }
}

void button2ISR()
{
    if (peripheralIsServer)
    {
        peripheralIsServer = false;
        printf("Peripheral is now GATT Client\r\n");
    }
    else
    {
        peripheralIsServer = true;
        printf("Peripheral is now GATT Server\r\n");
    }
}

/*****************************************************************************/
/* Security                                                                  */
/*****************************************************************************/

void securityInitiated(Gap::Handle_t, bool, bool, SecurityManager::SecurityIOCapabilities_t)
{
    printf("Security started\r\n");
}

void securityCompleted(Gap::Handle_t, SecurityManager::SecurityCompletionStatus_t)
{
    printf("Security completed\r\n");
}

void linkSecured(Gap::Handle_t, SecurityManager::SecurityMode_t)
{
    printf("Link secured\r\n");
}

void contextStored(Gap::Handle_t)
{
    printf("Context stored\r\n");
}

void passkeyDisplay(Gap::Handle_t, const SecurityManager::Passkey_t)
{
    printf("Display passkey\r\n");
}

/*****************************************************************************/
/* App start                                                                 */
/*****************************************************************************/
Timeout timeout;

void timeoutHandler()
{
    printf("timeout\r\n");
}

void commandInterpreter(void)
{
    if (!strcmp((const char*)buffer, "start")){
        button1ISR();
        bufferIndex = 0;
        memset(buffer, 0, strlen((char*)buffer));
    }
}

void serialHandler(void)
{
    char input = console.getc();
    if (input != '\n' && input != '\r'){
        buffer[bufferIndex++] = input;
    }
    commandInterpreter();
}

void app_start(int, char *[])
{
    buffer = (uint8_t*)malloc(24);
    memset(buffer, 0, 24);
    console.attach(serialHandler);
    printf("{{end}}\r\n");
    // setup buttons
    // button1.mode(PullUp);
    // Delay for initial pullup to take effect
    wait(.01);
    // button1.fall(button1ISR);

    // button2.mode(PullUp);
    // Delay for initial pullup to take effect
    wait(.01);
    // button2.fall(button2ISR);

    /*************************************************************************/
    /*************************************************************************/
    /* bluetooth le */
    ble.init();

    // security
    ble.securityManager().onSecuritySetupInitiated(securityInitiated);
    ble.securityManager().onSecuritySetupCompleted(securityCompleted);
    ble.securityManager().onLinkSecured(linkSecured);
    ble.securityManager().onSecurityContextStored(contextStored);
    ble.securityManager().onPasskeyDisplay(passkeyDisplay);

    // connection status handlers
    ble.gap().onConnection(whenConnected);
    ble.gap().onDisconnection(whenDisconnected);

    // set preferred connection parameters to lowest latency
    Gap::ConnectionParams_t fast;
    ble.gap().getPreferredConnectionParams(&fast);
    fast.minConnectionInterval = 16; // 20 ms
    fast.maxConnectionInterval = 32; // 40 ms
    fast.slaveLatency = 0;
    ble.gap().setPreferredConnectionParams(&fast);

    /* construct advertising beacon */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED|GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME, (const uint8_t *) DEVICE_NAME, sizeof(DEVICE_NAME) - 1);

    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, uuid.getBaseUUID(), uuid.getLen());

    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(200);

    // Apple uses device name instead of beacon name
    ble.gap().setDeviceName((const uint8_t*) DEVICE_NAME);

    // ble setup complete - start advertising
    ble.gap().startAdvertising();


    /*************************************************************************/
    /*************************************************************************/
    bts.init(uuid, SecurityManager::SECURITY_MODE_ENCRYPTION_OPEN_LINK);

#if 0
    writeBlock.push_back(&writeBlockFragment2);
    writeBlock.push_back(&writeBlockFragment3);
    writeBlock.push_back(&writeBlockFragment4);
#endif

    // set callback functions
    bts.setWriteAuthorizationCallback(blockServerWriteHandler, &receiveBlock);
    bts.setReadAuthorizationCallback(blockServerReadHandler);

    // blink led
    ticker.attach(periodicCallback, 1.0);

    printf("BlockTransfer: %s %s %d\r\n", __DATE__, __TIME__, MAX_INDEX_SET_SIZE);
}

/*****************************************************************************/
/* Compatibility                                                             */
/*****************************************************************************/

#if defined(YOTTA_MINAR_VERSION_STRING)
/*********************************************************/
/* Build for mbed OS                                     */
/*********************************************************/
/*
    Task for sending notification.

    This signals the client to perform a read.
*/
void sendNotificationTask()
{
    ble_error_t result = bts.updateCharacteristicValue((const uint8_t*)"hello", 5);

    // retry
    if (result != BLE_ERROR_NONE)
    {
        minar::Scheduler::postCallback(sendNotificationTask);
    }
}

void signalReady()
{
    minar::Scheduler::postCallback(sendNotificationTask);
}

#else
/*********************************************************/
/* Build for mbed Classic                                */
/*********************************************************/

void signalReady()
{
    sendHello = true;
}

int main(void)
{
    app_start(0, NULL);

    for(;;)
    {
        // send notification outside of interrupt context
        if (sendHello)
        {
            ble_error_t result = bts.updateCharacteristicValue((const uint8_t*)"hello", 5);

            if (result == BLE_ERROR_NONE)
            {
                sendHello = false;
            }
        }

        ble.waitForEvent();
    }
}
#endif
