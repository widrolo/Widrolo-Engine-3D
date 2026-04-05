@page scripting Getting Started

# Scripting

## Language
The only supported language is C++. The verison currently used is C++26 using GCC.

In the future, there will be Lua support, but that is far into the future. Its just C++ for now.

## Framework
The framework that Widrolo Engine provides is meant to make the development as easy as possible. 

When you want to make game logic, you mostly rely on components. Making a component is quite simple, as you just inherit the WEngine::Component class. 

When you want to make UI, whether for debugging or game UI, you inherit the WEngine::Widget class. Knowlege of ImGui is recommended for this one.