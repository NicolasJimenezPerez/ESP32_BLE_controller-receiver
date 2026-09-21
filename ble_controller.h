#ifndef __BLE_CONTROLLER_H__
#define __BLE_CONTROLLER_H__


#include <inttypes.h>

#include <string>

#include <Bounce2.h>
#include <BleGamepad.h>

#include "ble_remote_axis.h"


/// @brief Enum to set the controller type
enum class BleControllerType
{
    JOYSTICK = 4,
    GAMEPAD = 5,
    MULTi_AXES = 8,
};


/// @brief Class to define the controller axis
class BleControllerAxis
{

protected:



    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                       Protected Variables                      ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief Variable to hold the axis name
    BleRemoteAxisName name;

    /// @brief Variable to hold the pin assigned to the axis
    uint8_t pin;

    /// @brief Minimum value read by the pin
    int16_t minValue;

    /// @brief Value of the pin in its resting position. If it has none, set restingValue to 0
    int16_t restingValue;

    /// @brief Maximum value read by the pin.
    int16_t maxValue;

    /// @brief Dead zone region size, centered in the resting value. Only applies if resting value is set
    int16_t deadZone;

    /// @brief Flag to enable the application of calibration to the axis values
    bool applyCalibration;


public:



    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                          Constructors                          ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief Constructor for the BleControllerAxis class
    /// @param name name of the axis
    /// @param pin Pin associated with the axis
    /// @param minValue Minimum value read by the pin
    /// @param maxValue Maximum value read by the pin
    /// @param restingValue Value of the pin in the resting position
    /// @param deadZone Dead zone size, centered in restingValue.
    /// @param applyCalibration Flag to enable the application of calibration to the axis values
    BleControllerAxis(const BleRemoteAxisName name, const uint8_t pin, const int16_t minValue, const int16_t maxValue, const int16_t restingValue = 0, const int16_t deadZone = 0, const bool applyCalibration = false) :
        name(name), pin(pin), minValue(minValue), maxValue(maxValue), restingValue(restingValue), 
        deadZone(deadZone), applyCalibration(applyCalibration) {}


    
    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                          Public Methods                        ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief Getter for the name
    /// @return The axis name
    BleRemoteAxisName getName() { return name; }
    
    /// @brief Getter for the pin
    /// @return The pin associated with the axis
    uint8_t getPin() { return pin; }

    /// @brief Getter for the minimum value
    /// @return The axis minimum value
    int16_t getMinValue() { return minValue; }

    /// @brief Getter for the resting value
    /// @return The axis resting value
    int16_t getRestingValue() { return restingValue; }

    /// @brief Getter for the maximum value
    /// @return Tthe axis maximum value
    int16_t getMaxValue() { return maxValue; }

    /// @brief Getter for the dead zone
    /// @return The axis dead zone size
    int16_t getDeadZone() { return deadZone; }

    /// @brief Getter for the apply calibration flag
    /// @return True if calibration must be applied to the axis
    bool getApplyCalibration() { return applyCalibration; }

};


/// @brief Struct for the controller configuration
struct BleControllerConfig
{
    /// @brief Variable to hold the controller name
    std::string name;

    /// @brief Variable to hold the manufacturer name
    std::string manufacturerName;

    /// @brief variable to hold the battery level displayed at the start of the connection
    uint8_t startingBatteryLevel;

    /// @brief Flag to set if the connection should be delayed
    bool delayAdvertising = false;


    /// @brief Variable to hold the controller type
    BleControllerType controllerType;

    /// @brief Variable to hold the vendor ID
    uint16_t vendorId = 0xFFFF;

    /// @brief Variable to hold the product ID
    uint16_t productId = 0xFFFF;

    /// @brief Variable to hold the RF power level of the controoller
    int8_t powerLevel = 3;

    /// @brief Variable to hold the model number of the controller
    std::string modelNumber;

    /// @brief Variable to hold the software revision of the controller
    std::string softwareRevision;

    /// @brief Variable to hold the serial number of the controller
    std::string serialNumber;

    /// @brief Variable to hold the firmware revision of the controller
    std::string firmwareRevision;

    /// @brief Variable to hold the hardware revision of the controller
    std::string hardwareRevision;


    /// @brief Vector to define the pins assigned to each button
    std::vector<uint8_t> ButtonPins = { static_cast<uint8_t>(NOT_A_PIN) };

    /// @brief Variable for the time for the debounce of the controller buttons
    uint8_t buttonDebounceTime = 10;
    

    /// @brief Vactor to define the axes of the controller
    std::vector<BleControllerAxis> axes;

    /// @brief Moving average window size for the axes filtering. Set to 1 to disable the moving average.
    uint8_t axesMovingAverageSize = 1;

    /// @brief Minimum value the axes can output
    int16_t axesMin = INT16_MIN;

    /// @brief Maximum value the axes can output
    int16_t axesMax = INT16_MAX;

    /// @brief X axis value of the inflection point of the axes calibration curve. Only applies to the axes that should have calibrarion apllied to them
    float axesCalCurveDiscPointX = 0.0f;

    /// @brief Y axis value of the inflection point of the axes calibration curve. Only applies to the axes that should have calibrarion apllied to them
    float axesCalCurveDiscPointY = 0.0f;
};


/// @brief Class for the Ble controller
class BleController
{

protected:



    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                       Protected Variables                      ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief variable to hold the button debouncer
    Bounce debouncer;

    /// @brief Variable to hold the Ble gamepad object
    BleGamepad controller;


    /// @brief Vector to hold the pins assigned to the buttons
    std::vector<uint8_t> buttonPins;

    /// @brief Vector to store the button states
    std::vector<bool> buttonStates;

    /// @brief Array to hold the axes index in the axes array
    uint8_t axesIndex[8];

    /// @brief Vector to hold the defined axes
    std::vector<BleControllerAxis> axes;


    /// @brief Minimum value the axes can output
    int16_t axesMin;

    /// @brief Maximum value the axes can output
    int16_t axesMax;
    
    /// @brief Vector to store the moving average buffers for each axis
    std::vector<std::vector<int16_t>> axesMovingAverageBuf;

    /// @brief Array to hold de index for the moving average of each axis
    uint8_t axesMovingAverageIndex[8];

    /// @brief X axis value of the inflection point of the axes calibration curve.
    float axesCalCurveDiscPointX;

    /// @brief The slope of the first line of the calibration
    float axesCalCurveFirstM;

    /// @brief The slope of the second line of the calibration
    float axesCalCurveSecondM;

    /// @brief The crossing point of the second line of the calibration with th Y axis
    float axesCalCurveSecondN;


public:



    ////////////////////////////////////////////////////////////////////////
    ////                                                                ////
    ////                         Public Methods                         ////
    ////                                                                ////
    ////////////////////////////////////////////////////////////////////////



    /// @brief Method to initialize the controller
    /// @param controllerConfig Controller configuration to be initialized with
    void init(const BleControllerConfig& controllerConfig);


    /// @brief Method to check the connection state of the controller
    /// @return True if the controller is connected
    bool checkConnection() { return controller.isConnected(); }


    /// @brief Getter for a button state
    /// @param buttonNumber The button number to get the state from, as its index on the ButtonPins Vector in the controller configuration
    /// @return True if the button is pressed
    bool getButtonState(uint8_t buttonNumber);
    
    /// @brief Setter for the state for a given button
    /// @param buttonNumber The button number to get the state from, as its index on the ButtonPins Vector in the controller configuration 
    /// @param isPressed The state of the button to be set
    void setButtonState(uint8_t buttonNumber, bool isPressed);

    /// @brief Method to update the state of a given button
    /// @param buttonNumber The button number to get the state from, as its index on the ButtonPins Vector in the controller configuration 
    void updateButtonState(uint8_t buttonNumber);

    /// @brief Method to update all the buttons in the controller
    void updateAllButtonStates();


    /// @brief Getter for the value of a given axis
    /// @param axisName The name of the axis to get the value from
    /// @return The value of the axis
    int16_t getAxisValue(BleRemoteAxisName axisName);
    
    /// @brief Setter for the value of a given axis
    /// @param axisName name of the axis to be set
    /// @param value Value to be set
    void setAxisValue(BleRemoteAxisName axisName, int16_t value);
    
    /// @brief Method to update a given axis value
    /// @param axisName The name of the axis to be updated
    void updateAxisValue(BleRemoteAxisName axisName);

    /// @brief Method to update all axes in the controller
    void updateAllAxesValues();


    /// @brief Method to update the state of the whole controller
    void update();


    /// @brief Method to send the state of the controller via bluetooth
    void sendReport();
};


#endif