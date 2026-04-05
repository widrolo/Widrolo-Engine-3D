#pragma once

#include <string>
#include <SDL3/SDL.h>
#include <Engine/Math/Vector.h>
#include <Engine/Types/Rendering/Color.h>

namespace WEngine
{
    /**
     * Types of input peripherals that can be connected to the engine.
     */
    enum class InputPeripheralType
    {
        Unknown,
        Controller,
        SteeringWheel,
    };

    /**
     * Vendors of input peripherals supported by the engine.
     */
    enum class InputPeripheralVendor
    {
        Unknown,

        // Controllers
        Sony,
        Microsoft,
        Nintendo, // not tested
        Valve, // not tested

        // Off-brand controllers
        Razer, // not tested
        EightBitDo, // not tested
        PowerA, // not tested
        Hori, // not tested
        Nacon, // not tested
        Turtlebeach,

        // Steering Wheels
        Logitech,
        Thrustmaster, // not tested
        Fanatech, // not tested

        Supported_Vendor_Count
    };

    extern std::string InputPeripheralVendor_Name[];

    /**
     * Supported features that can be found on a steering wheel.
     */
    enum class WheelFeatures : uint8
    {
        SteeringWheel       = 1 << 0,
        ThrottleAndBrake    = 1 << 1,
        Clutch              = 1 << 2,
        SequentialShifter   = 1 << 3,
        H_Shifter           = 1 << 4,
        ParkingBrake        = 1 << 5,
        ForceFeedback       = 1 << 6,
        RPMLed              = 1 << 7,
    };

    /**
     * Types of controllers supported by the system.
     */
    enum class ControllerType
    {
        Unknown,
        Xbox,
        PlayStation,
        Nintendo
    };

    // This is scuffed, but it has to be for compatibility with the
    // input system. This is a "to each their own" type of thing,
    // where each peripheral can have a different amount of buttons.
    // Its better not to touch it unless you are WEngine::InputHandler
    // itself.
    /**
     * Struct representing the buttons on an input peripheral. Its better not to touch it unless you are WEngine::InputHandler
     * itself.
     */
    struct InputPeripheralButtons
    {
        bool* current;  ///< Current state of the buttons
        bool* last;     ///< Previous state of the buttons
    };

    /**
     * Struct representing a touchpad on an input peripheral.
     */
    struct Touchpad
    {
        // well technically we could have multiple fingers
        // but whatever for now...

        bool fingerOnTouchpad;  ///< Indicates if a finger is currently on the touchpad
        Vector2 fingerPosition; ///< Position of the finger on the touchpad
    };

    /**
     * Struct representing the force feedback state of a steering wheel.
     */
    struct WheelForceFeedbackState
    {
        float32 springStrength;     ///< Strength of the spring force
        float32 damperStrength;     ///< Strength of the damper force
        float32 frictionStrength;   ///< Strength of the friction force
    };

    /**
     * Struct representing a controller input peripheral.
     */
    struct Controller
    {
        Vector2 leftStick;              ///< Position of the left stick
        Vector2 rightStick;             ///< Position of the right stick

        bool hasTriggerRumble;          ///< Indicates if the controller supports trigger rumble
        bool hasTriggerResistance;      ///< Indicates if the controller supports trigger resistance
        float32 leftTrigger;            ///< Position of the left trigger
        float32 rightTrigger;           ///< Position of the right trigger

        uint8 touchpadCount;            ///< Number of touchpads on the controller
        Touchpad* touchpads;            ///< Array of touchpads on the controller

        bool hasLED;                    ///< Indicates if the controller has a LED
        Color ledColor;                 ///< Color of the LED
        ControllerType controllerType;  ///< Type of the controller

        bool hasRumble;                 ///< Indicates if the controller supports rumble
    };

    /**
     * Struct representing a steering wheel input peripheral.
     */
    struct SteeringWheel
    {
        uint8 wheelFeatures;                        ///< Features supported by the steering wheel
        float wheel;                                ///< Position of the steering wheel (-1 to 1)

        float throttle;                             ///< Position of the throttle pedal
        float brake;                                ///< Position of the brake pedal
        float clutch;                               ///< Position of the clutch pedal

        int8 hShifterPosition;                      ///< Position of the H-shifter (-1 = R, 0 = N, 1 to 7 = D)
        uint8 sequentialShifterState;               ///< State of the sequential shifter (0 = nothing, 1 = down, 2 = up, 3 = both)
        uint8 lastSequentialShifterState;           ///< Previous state of the sequential shifter
        bool parkingBrakeState;                     ///< State of the parking brake

        WheelForceFeedbackState forceFeedbackState; ///< Force feedback state of the steering wheel
    };

    /**
     * Struct representing an input peripheral connected to the system.
     */
    struct InputPeripheral
    {
        InputPeripheralVendor vendor;           ///< Vendor of the input peripheral
        std::string peripheralName;             ///< Name of the input peripheral
        std::string vendorName;                 ///< Name of the vendor
        SDL_JoystickID joyID;                   ///< ID of the joystick associated with the input peripheral
        SDL_Joystick* joystick;                 ///< Pointer to the joystick object

        uint16 usbVendorID;                     ///< USB vendor ID of the input peripheral
        uint16 usbProductID;                    ///< USB product ID of the input peripheral

        // In some circumstances  we will have to do extra processing.
        // Dualsenses in dev kit mode can sometimes work differently (Unity i
        // heard has problems in that regard).
        // Im suspecting that it comes down to engines comparing product IDs
        // instead of vendor IDs, but i dont have dev kit to test it out.
        bool isDevKit;                          ///< Indicates if the input peripheral is in dev kit mode

        InputPeripheralButtons buttons;         ///< Buttons on the input peripheral
        InputPeripheralType type;               ///< Type of the input peripheral
        union
        {
            Controller* controller;             ///< Pointer to the controller object (if the input peripheral is a controller)
            SteeringWheel* steeringWheel;       ///< Pointer to the steering wheel object (if the input peripheral is a steering wheel)
        };
    };
}