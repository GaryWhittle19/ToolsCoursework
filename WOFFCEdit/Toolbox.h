#pragma once

class Toolbox
{
public:
	static float Clamp(float x, float min, float max) {
		if (x < min)
			x = min;
		else if (x > max)
			x = max;
		else
			x = x;

		return x;
	}

	static float MappedClamp(float x, float min, float max, float mapped_min, float mapped_max) {
		float output = mapped_min + ((mapped_max - mapped_min) / (max - min)) * (x - min);
		return output;
	}

    static void LogOutput(std::string msg, std::string name = "Log") {
        msg = name + msg + "\n";
        std::wstring ws = std::wstring(msg.begin(), msg.end());
        LPCWSTR ls = ws.c_str();
        OutputDebugString(ls);
    }

    static void LogPoint(DirectX::SimpleMath::Vector3 _point, std::string _name = "Point") {
        LogOutput(
            "x: " + std::to_string(_point.x) +
            " y: " + std::to_string(_point.y) +
            " z: " + std::to_string(_point.z),
            _name
        );
    }

    static void LogRay(DirectX::SimpleMath::Ray _ray, std::string _name = "Ray") {
        LogOutput(
            "position { x: " + std::to_string(_ray.position.x) + " y: " + std::to_string(_ray.position.y) + " z: " + std::to_string(_ray.position.z) +
            " | direction { x: " + std::to_string(_ray.direction.x) + " y: " + std::to_string(_ray.direction.y) + " z: " + std::to_string(_ray.direction.z),
            _name
        );
    }
};