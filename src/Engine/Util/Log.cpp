#include "Log.h"

#include <iostream>
#include <cstdarg>
#include <iomanip>
#include <Engine/Core/System/OS.h>
#include <filesystem>
#include <Engine/Types/CommonTypes.h>

#include <Engine/Math/Vector.h>

#include "Time.h"

using namespace WEngine;

void WLog::SetConsoleMessage()
{
    OS::SetConsoleColor(15);
}

void WLog::SetConsoleInfo()
{
    OS::SetConsoleColor(11);
}

void WLog::SetConsoleWarning()
{
    OS::SetConsoleColor(14);
}

void WLog::SetConsoleError()
{
    OS::SetConsoleColor(12);
}

void WLog::SetConsoleSuccess()
{
    OS::SetConsoleColor(10);
}

void WLog::PrintInfo(const std::source_location& location)
{
    TimePoint time = Time::GetLocalTime();

    // Convert const char* to std::string
    std::string filePath(location.file_name());

    // Use std::filesystem::path to extract filename

    std::filesystem::path p(filePath);
    std::string filename = p.filename().string();

    std::cout << "[" << std::setfill('0') << std::setw(2) << time.Hour
        << ":" << std::setfill('0') << std::setw(2) << time.Minute
        << ":" << std::setfill('0') << std::setw(2) << time.Second
        << " " << filename << ":" << location.line() << "] ";
}