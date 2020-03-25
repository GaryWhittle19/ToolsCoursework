#pragma once
#include <cstdint>

// Struct for when requiring more complex functionality
struct Key
{
	bool down;		// 
	bool released;	// 
	bool pressed;	// 
};

using std::int32_t;

struct InputCommands
{
	// For control in all six directions
	bool forward;
	bool back;
	bool right;
	bool left;
	bool up;
	bool down;
	// Mouse buttons
	bool mouseLeft;
	bool mouseRight;
	// Editor functionality 
	bool sprint;
	bool wireframe_toggle;
	bool edit_toggle;
	bool ray_toggle;
	// Mouse coordinates
	int x, y;
};

class InputProcessor {
public:
	// Key array
	Key keys[256];

	// Getters
	bool IsKeyDown(uint32_t keycode)
	{
		return keys[keycode].down;
	}
	bool WasKeyPressed(uint32_t keycode)
	{
		return keys[keycode].pressed;
	}
	bool WasKeyReleased(uint32_t keycode)
	{
		return keys[keycode].released;
	}

	// Setter
	void SetKey(uint32_t keyCode, bool isDown, bool isReleased, bool isPressed)
	{
		keys[keyCode].down = isDown;
		keys[keyCode].released = isReleased;
		keys[keyCode].pressed = isPressed;
	}

	// Reset key statuses
	void ResetKeys()
	{
		for (int i = 0; i < 256; i++) // Loop through key array - NOTE: HARD CODED 
		{
			keys[i].released = false;
			keys[i].pressed = false;
		}
	}
};