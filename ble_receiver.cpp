// Flag to select if the library is used as controller or receiver.
// This is needed because PlatformIO compiles all libraries files before 
// checking if they are included in the fource tree or not. This forces
// to also include all unused dependencies uncluded by those files.
#ifdef __BLE_RECEIVER__


#include "ble_reciever.h"


#include <Arduino.h>
#include <esp_log.h>


constexpr char BLE_RECEIVER_LOG_TAG[] = "BLE_RECEIVER";


void BleReceiver::ScanForPeerCallbacks::onResult(const NimBLEAdvertisedDevice* p_advertisedDevice)
{
    if ("*" != receiver.peerInfo.Name && receiver.peerInfo.Name == p_advertisedDevice->getName())
    {
        NimBLEDevice::getScan()->stop();
        receiver.p_advertisedDevice = p_advertisedDevice;
        receiver.peerFound = true;
    }
}


void BleReceiver::ScanForPeerCallbacks::onScanEnd(const NimBLEScanResults& results, int reason)
{
    NimBLEDevice::getScan()->start(receiver.scanningTimeoutMs, false, true);
}


void BleReceiver::ClientCallbacks::onDisconnect(NimBLEClient* pClient, int reason)
{
    receiver.peerFound = false;
    NimBLEDevice::getScan()->start(receiver.scanningTimeoutMs, false, true);
}


bool BleReceiver::connectToPeer()
{
    if (!peerFound)
        return false;

    if(isConnected())
        return true;
    
    
    p_client = nullptr;

    // Reuse a client if it exists
    if (NimBLEDevice::getCreatedClientCount())
    {
        // In case of reconnection
        p_client = NimBLEDevice::getClientByPeerAddress(p_advertisedDevice->getAddress());
        if (p_client)
        {
            if (!p_client->connect(p_advertisedDevice, false))
            {
                ESP_LOGI(BLE_RECEIVER_LOG_TAG, "Reconnect failed\n");
                return false;
            }
        } 
        else
            p_client = NimBLEDevice::getDisconnectedClient();
    }
    
    if (!p_client)
    {
        if (NimBLEDevice::getCreatedClientCount() >= MYNEWT_VAL(BLE_MAX_CONNECTIONS))
        {
            ESP_LOGI(BLE_RECEIVER_LOG_TAG, "Max clients reached - no more connections available\n");
            return false;
        }

        p_client = NimBLEDevice::createClient();
        p_client->setClientCallbacks(&clientCallbacks, false);
        p_client->setConnectionParams(12, 12, 0, 150);
        p_client->setConnectTimeout(5000);

        if (!p_client->connect(p_advertisedDevice))
        {
            NimBLEDevice::deleteClient(p_client);
            p_client = nullptr;
            ESP_LOGI(BLE_RECEIVER_LOG_TAG, "Failed to connect, deleted client\n");
            return false;
        }
    }

    if (!p_client->isConnected())
    {
        if (!p_client->connect(p_advertisedDevice))
        {
            ESP_LOGI(BLE_RECEIVER_LOG_TAG, "Failed to connect, deleted client\n");
            return false;
        }
    }

    const NimBLERemoteService *p_serv = p_client->getService("1812");
    if (p_serv)
    {
        // @TODO: Leer HID_MAP en service 1812, characteristic 2A4B
        
        NimBLERemoteCharacteristic *p_char = p_serv->getCharacteristic("2A4D");
        if (p_char)
            p_dataChar = p_char;
        else
        {
            ESP_LOGI(BLE_RECEIVER_LOG_TAG, "Characteristic 2A4D not found\n");
            return false;
        }
    }
    else
    {
        ESP_LOGI(BLE_RECEIVER_LOG_TAG, "Service 1812 not found\n");
        return false;
    }

    return true;
}


void BleReceiver::init(std::string name, uint8_t powerLevel)
{
    NimBLEDevice::init(name);
    NimBLEDevice::setPower(powerLevel);
}


void BleReceiver::setPeer(const BleReceiverPeerInfo& peerInfo)
{
    this->peerInfo = peerInfo;
    isPeerInfoSet = true;
}


bool BleReceiver::scanForPeer()
{
    if (!isPeerInfoSet)
        return false;

    NimBLEScan *p_scan = NimBLEDevice::getScan();
    p_scan->setScanCallbacks(&scanForPeerCallbacks, false);

    // Set scan interval (how often) and window (how long) in milliseconds
    p_scan->setInterval(100);
    p_scan->setWindow(100);

    p_scan->setActiveScan(true);

    p_scan->start(scanningTimeoutMs);

    return true;
}


// @TODO: 
bool BleReceiver::isConnected()
{
    return (nullptr != p_client) && (p_client->isConnected());
}


void BleReceiver::updateData()
{
    if ((nullptr != p_dataChar) && (p_dataChar->canRead()))
    {
        const uint8_t *p_data = p_dataChar->readValue().data();

        // Update buttons states
        for (uint8_t i = 0; i < buttonIndex.size(); ++i)
            if (0xFF != buttonIndex[i])
                buttonStates[buttonIndex[i]] = (p_data[buttonIndex[i] / 8] >> (i % 8)) & 0x01;
    
        // Update axes values
        uint8_t axisStartingIndex = (buttonIndex.size() + 7) / 8;
        for (uint8_t i = 0; i < 8; ++i)
            if (0xFF != axesIndex[i])
                axesValues[axesIndex[i]] = ((int16_t *)(p_data + axisStartingIndex))[axesIndex[i]];
    }
}


bool BleReceiver::getButtonState(uint8_t buttonNumber)
{
    // If the button number is greater than the number of buttons
    if (buttonIndex.size() - 1 < buttonNumber)
        return false;

    // If the button is not in use
    if (0xFF == buttonIndex[buttonNumber])
        return false;

    return buttonStates[buttonIndex[buttonNumber]];
}


// @TODO: 
int16_t BleReceiver::getAxisMinValue()
{    
    return INT16_MIN;
}


// @TODO: 
int16_t BleReceiver::getAxisMaxValue()
{
    return INT16_MIN;
}


int16_t BleReceiver::getAxisValue(BleRemoteAxisName axisName)
{
    // If the axis is no in use
    if (0xFF == axesIndex[static_cast<uint8_t>(axisName)])
        return false;

    return axesValues[axesIndex[static_cast<uint8_t>(axisName)]];
}


#endif