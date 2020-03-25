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
};