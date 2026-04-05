#include "Color.h"

using namespace WEngine;

const Color Color::White = Color(255, 255, 255, 255);
const Color Color::Black = Color(0, 0, 0, 255);
const Color Color::Red   = Color(255, 0, 0, 255);
const Color Color::Green = Color(0, 255, 0, 255);
const Color Color::Blue  = Color(0, 0, 255, 255);
Color::Color(uint32 rgba) {
    red = rgba & 0xff;
    green = (rgba >> 8) & 0xff;
    blue = (rgba >> 16) & 0xff;
    alpha = (rgba >> 24) & 0xff;
}
