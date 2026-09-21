#include "ble_controller.h"


void BleController::init(const BleControllerConfig& controllerConfig)
{
    controller = BleGamepad(controllerConfig.name, controllerConfig.manufacturerName,
        controllerConfig.startingBatteryLevel, controllerConfig.delayAdvertising);
    
    // Configure gamepad
    BleGamepadConfiguration gamepadConfig;

    gamepadConfig.setAutoReport(false);
	gamepadConfig.setControllerType(static_cast<uint8_t>(controllerConfig.controllerType));
	gamepadConfig.setVid(controllerConfig.vendorId);
	gamepadConfig.setPid(controllerConfig.productId);
	gamepadConfig.setTXPowerLevel(controllerConfig.powerLevel);

	gamepadConfig.setModelNumber(controllerConfig.modelNumber.c_str());
	gamepadConfig.setSoftwareRevision(controllerConfig.softwareRevision.c_str());
	gamepadConfig.setSerialNumber(controllerConfig.serialNumber.c_str());
	gamepadConfig.setFirmwareRevision(controllerConfig.firmwareRevision.c_str());
	gamepadConfig.setHardwareRevision(controllerConfig.hardwareRevision.c_str());

	gamepadConfig.setButtonCount(controllerConfig.ButtonPins.size());
    
    gamepadConfig.setAxesMin(axesMin);
	gamepadConfig.setAxesMax(axesMax);
    
    buttonPins = controllerConfig.ButtonPins;
    buttonStates = std::vector<bool>(buttonPins.size(), false);

    for (int i = 0; i < buttonPins.size(); ++i)
        if (NOT_A_PIN != buttonPins[i])
            pinMode(buttonPins[i], INPUT_PULLUP);
    
    axes = controllerConfig.axes;
    for (uint8_t i = 0; i < 8; ++i)
        axesIndex[i] = 0xFF;
    for (uint8_t i = 0; i < axes.size(); ++i)
        axesIndex[static_cast<uint8_t>(axes[i].getName())] = i;
    gamepadConfig.setWhichAxes(0xFF != axesIndex[0], 0xFF != axesIndex[1], 0xFF != axesIndex[2], 0xFF != axesIndex[3], 0xFF != axesIndex[4], 0xFF != axesIndex[5], 0xFF != axesIndex[6], 0xFF != axesIndex[7]);

    axesMovingAverageBuf = std::vector<std::vector<int16_t>>(axes.size(), std::vector<int16_t>(controllerConfig.axesMovingAverageSize, 0));

    for (int i = 0; i < axes.size(); ++i)
        pinMode(axes[i].getPin(), ANALOG);

    axesMin = controllerConfig.axesMin;
    axesMax = controllerConfig.axesMax;
    axesCalCurveDiscPointX = controllerConfig.axesCalCurveDiscPointX;
    axesCalCurveFirstM = controllerConfig.axesCalCurveDiscPointY/axesCalCurveDiscPointX;
    axesCalCurveSecondM = (1 - controllerConfig.axesCalCurveDiscPointY) / (1 - axesCalCurveDiscPointX);
    axesCalCurveSecondN = (controllerConfig.axesCalCurveDiscPointY - axesCalCurveDiscPointX * axesCalCurveSecondM);

    controller.begin(&gamepadConfig);

    // Configure button debounce
    if (0 != controllerConfig.buttonDebounceTime)
    {
        for (uint8_t i = 0; i < controllerConfig.ButtonPins.size(); ++i)
            debouncer.attach(controllerConfig.ButtonPins[i]);

        debouncer.interval(controllerConfig.buttonDebounceTime);
    }
}


bool BleController::getButtonState(uint8_t buttonNumber)
{
    // If the button number is bigger thand the number of buttons
    if (buttonPins.size() < buttonNumber)
        return false;

    // If the button is not used
    if (NOT_A_PIN == buttonPins[buttonNumber])
        return false;
    
    return buttonStates[buttonNumber];
}


void BleController::setButtonState(uint8_t buttonNumber, bool isPressed)
{
    // If the button number is bigger thand the number of buttons
    if (buttonPins.size() < buttonNumber)
        return;

    // If the button is not used
    if (NOT_A_PIN == buttonPins[buttonNumber])
        return;

    // Set the pin value
    buttonStates[buttonNumber] = isPressed;
    if (isPressed)
        controller.press(buttonNumber + 1);
    else
        controller.release(buttonNumber + 1);
}


void BleController::updateButtonState(uint8_t buttonNumber)
{
    // If the button number is bigger thand the number of buttons
    if (buttonPins.size() < buttonNumber)
        return;

    // If the button is not used
    if (NOT_A_PIN == buttonPins[buttonNumber])
        return;

    setButtonState(buttonNumber, !digitalRead(buttonPins[buttonNumber]));
}


void BleController::updateAllButtonStates()
{
    for (uint8_t i = 0; i < buttonPins.size(); ++i)
        updateButtonState(i);
}


int16_t BleController::getAxisValue(BleRemoteAxisName axisName)
{
    uint8_t axisIndex = axesIndex[static_cast<uint8_t>(axisName)];

    // If axis is not defined
    if (0xFF == axisIndex)
        return 0;

    int16_t val = 0;
    for (uint8_t i = 0; i < axesMovingAverageBuf[0].size(); ++i)
            val += axesMovingAverageBuf[axesIndex[static_cast<uint8_t>(axisName)]][i];

    return val;
}


void BleController::setAxisValue(BleRemoteAxisName axisName, int16_t value)
{
    uint8_t axisIndex = axesIndex[static_cast<uint8_t>(axisName)];

    // If axis is not defined
    if (0xFF == axisIndex)
        return;

    BleControllerAxis& axis = axes[axisIndex];
    value = static_cast<int16_t>((value > 0 ? value / static_cast<float>(axis.getMaxValue()) * axesMax : value / static_cast<float>(axis.getMinValue()) * -1 * axesMin) - (axis.getRestingValue() ? 0 : axesMin));

    for (uint8_t i = 0; i < axesMovingAverageBuf[0].size(); ++i)
        axesMovingAverageBuf[axisIndex][i] = value;
}


void BleController::updateAxisValue(BleRemoteAxisName axisName)
{
    uint8_t axisIndex = axesIndex[static_cast<uint8_t>(axisName)];

    // If the axis is not enabled
    if (0xFF == axisIndex)
        return;
        
    // Get the axis
    BleControllerAxis& axis = axes[axisIndex];

    // Read the value from the axis and set the max, min and resting values
    int16_t val = analogRead(axis.getPin()) - axis.getRestingValue();

    // Force value to ranges and apply dead zone or scalate it to the controller axes range
    if (!axis.getRestingValue() && (axis.getDeadZone() > abs(val)))
        val = 0;
    else if (axis.getMaxValue() <= val)
        val = axesMax;
    else if (axis.getMinValue() >= val)
        val = axesMin;
    else
        val = static_cast<int16_t>((val > 0 ? val / static_cast<float>(axis.getMaxValue()) * axesMax : val / static_cast<float>(axis.getMinValue()) * axesMin) - (axis.getRestingValue() ? 0 : axesMin));

    // If calibration correction has to be applied
    if (axis.getApplyCalibration())
        if ((static_cast<int16_t>(axesCalCurveDiscPointX * axesMin) < val) && (static_cast<int16_t>(axesCalCurveDiscPointX * axesMax) > val))
            axesMovingAverageBuf[axisIndex][axesMovingAverageIndex[axisIndex]] = static_cast<int16_t>(axesCalCurveFirstM * val) / static_cast<float>(axesMovingAverageBuf[0].size());
        else
            axesMovingAverageBuf[axisIndex][axesMovingAverageIndex[axisIndex]] = static_cast<int16_t>(axesCalCurveSecondM * static_cast<float>(val) + (val > 0 ? 1 : -1) * axesCalCurveSecondN * (val > 0 ? axesMax : axesMin)) / static_cast<float>(axesMovingAverageBuf[0].size());
    else
        axesMovingAverageBuf[axisIndex][axesMovingAverageIndex[axisIndex]] = val;
        
    axesMovingAverageIndex[axisIndex] = ++axesMovingAverageIndex[axisIndex] % axesMovingAverageBuf[0].size();
}


void BleController::updateAllAxesValues()
{
    for (uint8_t i = 0; i < axes.size(); ++i)
        updateAxisValue(axes[i].getName());
}


void BleController::update()
{
    updateAllButtonStates();
    updateAllAxesValues();
}


void BleController::sendReport()
{
    // For every defined button set the state
    for (uint8_t i = 0; i < buttonStates.size(); ++i)
    {
        if (NOT_A_PIN != buttonPins[i])
            if (buttonStates[i])
                controller.press(i);
            else
                controller.release(i);
    }
        

    // For every axis
    for (uint8_t i = 0; i < axes.size(); ++i)
    {
        // Get the axis values from the moving average
        int16_t val = getAxisValue(static_cast<BleRemoteAxisName>(i));        

        // Load values into the controller
        switch (axes[i].getName())
        {
        case BleRemoteAxisName::Lx:
            controller.setX(val);
        break;

        case BleRemoteAxisName::Ly:
            controller.setY(val);
        break;

        case BleRemoteAxisName::Lz:
            controller.setZ(val);
        break;

        case BleRemoteAxisName::Rx:
            controller.setRX(val);
        break;

        case BleRemoteAxisName::Ry:
            controller.setRY(val);
        break;

        case BleRemoteAxisName::Rz:
            controller.setRZ(val);
        break;

        case BleRemoteAxisName::Slider1:
            controller.setSlider1(val);
        break;

        case BleRemoteAxisName::Slider2:
            controller.setSlider2(val);
        break;
        } 
    }

    // Send the controller values
    controller.sendReport();
}