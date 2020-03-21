#pragma once
#include <Windows.h>
#include <iostream>

class Debug
{
public:
    static void Out(std::string msg, std::string name = "Log") {
        msg = name + msg + "\n";
        std::wstring ws = std::wstring(msg.begin(), msg.end());
        LPCWSTR ls = ws.c_str();
        OutputDebugString(ls);
    }
};