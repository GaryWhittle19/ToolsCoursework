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

    static void Log(DirectX::SimpleMath::Vector3 _point, std::string _name = "Point") {
        Debug::Out(
            "x: " + std::to_string(_point.x) +
            " y: " + std::to_string(_point.y) +
            " z: " + std::to_string(_point.z),
            _name
            );
    }

    static void Log(DirectX::SimpleMath::Ray _ray, std::string _name = "Ray") {
        Debug::Out(
            "position { x: " + std::to_string(_ray.position.x) + " y: " + std::to_string(_ray.position.y) + " z: " + std::to_string(_ray.position.z) +
            " | direction { x: " + std::to_string(_ray.direction.x) + " y: " + std::to_string(_ray.direction.y) + " z: " + std::to_string(_ray.direction.z),
            _name
            );
    }
};