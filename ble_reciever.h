// Flag to select if the library is used as controller or receiver.
// This is needed because PlatformIO compiles all libraries files before 
// checking if they are included in the fource tree or not. This forces
// to also include all unused dependencies uncluded by those files.
#ifdef __BLE_RECEIVER__

#ifndef __BLE_RECIEVER_H__
#define __BLE_RECIEVER_H__


#include <inttypes.h>

#include <string>
#include <vector>

#include <NimBLEDevice.h>

#include "ble_remote_axis.h"


/// @brief Struct to hold the peer information
struct BleReceiverPeerInfo
{

    /// @brief Variable to store the name of the peer to connect to
    std::string Name = "";

    /// @brief Variable to store the name of the manufacturer of the peer to connecto to
    std::string manufacturerName = "";

    /// @brief Variable to store the serial number of the peer to connect to
    std::string serialNumber = "";
    
};


/// @brief Class to handle the BLE receiver capabilities
class BleReceiver
{

private:



    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                         Private Classes                        ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief Class to handle scanPeer callbacks
    class ScanForPeerCallbacks : public NimBLEScanCallbacks
    {

    private:



        ////////////////////////////////////////////////////////////////////////
        ////                                                                ////
        ////                        Private Variables                       ////
        ////                                                                ////
        ////////////////////////////////////////////////////////////////////////



        /// @brief Refecence to the paren receiver 
        BleReceiver& receiver;


    public:



        ////////////////////////////////////////////////////////////////////////
        ////                                                                ////
        ////                           Constructor                          ////
        ////                                                                ////
        ////////////////////////////////////////////////////////////////////////



        /// @brief Constructor for the ScanForPeerCallbacks class
        /// @param receiver Reference to the parent receiver
        ScanForPeerCallbacks(BleReceiver& receiver) : receiver(receiver) {}



        ////////////////////////////////////////////////////////////////////////
        ////                                                                ////
        ////                          Public Methods                        ////
        ////                                                                ////
        ////////////////////////////////////////////////////////////////////////


        /// @brief Method triggered on scanning result
        /// @param advertisedDevice Pointer to a NimBLE advertised device
        void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;

        /// @brief Method triggered on scan end
        /// @param results The result of the ended scan
        /// @param reason Reason for the end of the scan
        void onScanEnd(const NimBLEScanResults& results, int reason) override;
    
    };

    /// @brief Class to handle the p_client callbacks
    class ClientCallbacks : public NimBLEClientCallbacks 
    {
    
    private:



        ////////////////////////////////////////////////////////////////////////
        ////                                                                ////
        ////                        Private Variables                       ////
        ////                                                                ////
        ////////////////////////////////////////////////////////////////////////



        /// @brief Reference to the parent receiver
        BleReceiver& receiver;


    public:



        ////////////////////////////////////////////////////////////////////////
        ////                                                                ////
        ////                           Constructor                          ////
        ////                                                                ////
        ////////////////////////////////////////////////////////////////////////



        /// @brief Constructor for the ClientCallbacks class
        /// @param receiver Reference to the parent receiver
        ClientCallbacks(BleReceiver& receiver) : receiver(receiver) {}



        ////////////////////////////////////////////////////////////////////////
        ////                                                                ////
        ////                         Private Methods                        ////
        ////                                                                ////
        ////////////////////////////////////////////////////////////////////////



        /// @brief Method triggered on NimBLE client connection
        /// @param pClient Pointer to the NimBLE client that connected
        void onConnect(NimBLEClient* pClient) override {};

        /// @brief Method triggered on NimBLE client disconnection
        /// @param pClient Pointer to the disconnected NimBLE client
        /// @param reason Reason the disconnection occurred
        void onDisconnect(NimBLEClient* pClient, int reason) override;
    
    };



    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                        Private Variables                       ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief Variable to store the scanning timeout in ms. Set to 0 to scan indefinitely
    uint32_t scanningTimeoutMs = 5000;

    // @TODO: Asignar los valores en función del HID MAP
    // Puede que esto sea redundante porque se manden todos los ejes y botones no usados
    /// @brief Array to store the button indices
    std::vector<uint8_t> buttonIndex = {0};

    // @TODO: Asignar los valores en función del HID MAP
    // Puede que esto sea redundante porque se manden todos los ejes y botones no usados
    /// @brief Array to store the axes indices
    uint8_t axesIndex[8] = {0, 1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};    

    // TODO: Inicializar a las longitudes correctas
    /// @brief vector to store the read axes values
    std::vector<int16_t> axesValues = std::vector<int16_t>(2);

    // TODO: Inicializar a las longitudes correctas
    /// @brief Vector to store the read button states
    std::vector<bool> buttonStates = std::vector<bool>(1);

    /// @brief Flag to check if the peer info is set
    bool isPeerInfoSet = false;

    /// @brief Fleg to check if the peer has been found
    bool peerFound = false;

    /// @brief Object to store the peer info
    BleReceiverPeerInfo peerInfo;    

    /// @brief Pointer to the NimBLE client of the controller
    NimBLEClient *p_client = nullptr;

    /// @brief Pointer to the received data
    NimBLERemoteCharacteristic *p_dataChar;

    /// @brief Pointer to a NimBLE advertised device
    const NimBLEAdvertisedDevice* p_advertisedDevice = nullptr;

    /// @brief Object to store the scan for peer callbacks
    ScanForPeerCallbacks scanForPeerCallbacks;

    /// @brief Object to store the client callback functions
    ClientCallbacks clientCallbacks;
    

public:



    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                           Constructor                          ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief BleReceiver class constuctor
    BleReceiver() : scanForPeerCallbacks(ScanForPeerCallbacks(*this)),
                    clientCallbacks(ClientCallbacks(*this)) {}



    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                         Public Methods                         ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief Receiver initialization
    /// @param name Name of the receiver
    /// @param powerLevel Power level of the receiber on startup
    void init(std::string name, uint8_t powerLevel = 3);

    /// @brief Setter for the information of the peer to connect to
    /// @param peerInfo Information of the peer
    void setPeer(const BleReceiverPeerInfo& peerInfo);

    /// @brief Connect to the peer with the given info
    /// @return False if peer is not set
    bool scanForPeer();

    /// @brief Method to connect to the peer
    /// @return False if failed
    bool connectToPeer();

    /// @brief Method to get if the receiver is connected
    /// @return True if conected
    bool isConnected();
    
    /// @brief Method to update the received data from the controller 
    void updateData();

    /// @brief Getter for the state of a given button
    /// @param buttonNumber Number of the button to read
    /// @return True if the button is pressed
    bool getButtonState(uint8_t buttonNumber);

    /// @brief Getter for the minimum value the axes can have
    /// @return The minimum value
    int16_t getAxisMinValue();

    /// @brief Getter for the maximum value the axes can have
    /// @return The maximum value
    int16_t getAxisMaxValue();

    /// @brief Getter for a given axis value
    /// @param axisName The name of the axis to get the value from
    /// @return The value of the axis
    int16_t getAxisValue(BleRemoteAxisName axisName);

};


#endif

#endif