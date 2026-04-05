#pragma once

#include <functional>

namespace WEngine
{
    /*
     *  This warrants a greater explanation.
     *
     *  The intent is to make it able to support any function passed in.
     *  Any job needs to have two structs to support the call.
     *
     *  For example:
     *  struct Job_Calculate_Args { ... };
     *  struct Job_Calculate_Out { ... };
     *  void* Calculate(void* argStruct);
     *
     *  Would be a valid declaration.
     */
    using Job = std::function<void*(void* argStruct)>;
}