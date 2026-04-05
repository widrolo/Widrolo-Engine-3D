#include "KeyCodes.h"

// After further thinking, this is stupid. I will put this on ice for now in
// hopes that its actually just a Dualsense thing, and not a common problem.

// Also, if the button is 255, then it will always be false if queried.
// Otherwise, they point to SDL_GamepadButton. It would be too long if
// I tried to write them all out by name, so I do it like this instead.

// for unknown vendors
uint8 WPadBtn_Generic[(uint8)WPadBtn::Pad_Button_Count] = {
    // face buttons
    0, 1, 2, 3,
    // selections
    4, 6,
    // shoulder and sticks
    7, 8, 9, 10, 100, 101,
    // D-Pad
    11, 12, 13, 14,
    // touchpad
    255,
    // back buttons
    255, 255, 255, 255
};

// playstation has square and triangle switched
uint8 WPadBtn_Playstation[(uint8)WPadBtn::Pad_Button_Count] = {
    // face buttons
    0, 1, 3, 2,
    // selections
    4, 6,
    // shoulder and sticks
    7, 8, 9, 10, 100, 101,
    // D-Pad
    11, 12, 13, 14,
    // touchpad
    20,
    // back buttons
    16, 17, 255, 255
};

uint8 WPadBtn_Xbox[(uint8)WPadBtn::Pad_Button_Count] = {
    // face buttons
    0, 1, 2, 3,
    // selections
    4, 6,
    // shoulder and sticks
    7, 8, 9, 10, 100, 101,
    // D-Pad
    11, 12, 13, 14,
    // touchpad
    255,
    // back buttons
    16, 17, 18, 19
};