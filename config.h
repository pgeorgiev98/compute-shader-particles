#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <vector>
#include <variant>
#include <string>
#include <sstream>

#define WINDOW_TITLE "Particles"

struct Config
{
	int width = 768, height = 768;
	bool enableVSync = true;

	// number of particles = countX * countY
	// Both must be multiples of 16
	int particleCountX = 1024;
	int particleCountY = 1024;

	float forceMultiplier = 30000.0;

	int maxParticlesOnPixel = 15;
	float colorRedMul = 1.0;
	float colorRedAdd = 0.0;
	float colorGreenMul = 1.0;
	float colorGreenAdd = 0.0;
	float colorBlueMul = 1.0;
	float colorBlueAdd = -1.0;

	float massMin = 0.75;
	float massMax = 1.25;

	float drag1 = 0.0;
	float drag2 = 0.0;
	float drag3 = 0.0;

	float minimumDistance = 10.0;

	enum {
		WALL_COLLISION_NONE = 0,
		WALL_COLLISION_STOP = 1,
		WALL_COLLISION_REFLECT = 2,
	};
	int wallCollisionAction = WALL_COLLISION_REFLECT;
	float collisionReflectSpeed = 0.5;

	typedef std::variant<bool *, int *, float *> V;
	std::vector<std::pair<std::string, V>> values = {
		{"width", &width},
		{"height", &height},
		{"enableVSync", &enableVSync},
		{"particleCountX", &particleCountX},
		{"particleCountY", &particleCountY},
		{"forceMultiplier", &forceMultiplier},
		{"maxParticlesOnPixel", &maxParticlesOnPixel},
		{"colorRedMul", &colorRedMul},
		{"colorRedAdd", &colorRedAdd},
		{"colorGreenMul", &colorGreenMul},
		{"colorGreenAdd", &colorGreenAdd},
		{"colorBlueMul", &colorBlueMul},
		{"colorBlueAdd", &colorBlueAdd},
		{"massMin", &massMin},
		{"massMax", &massMax},
		{"drag1", &drag1},
		{"drag2", &drag2},
		{"drag3", &drag3},
		{"minimumDistance", &minimumDistance},
		{"wallCollisionAction", &wallCollisionAction},
		{"collisionReflectSpeed", &collisionReflectSpeed},
	};

	V find(std::string name) const
	{
		for (auto p : values)
			if (p.first == name)
				return p.second;
		throw "Invalid config value name: " + name;
	}

	std::string getValue(std::string name) const
	{
		using namespace std;
		auto v = find(name);
		if (holds_alternative<bool *>(v))
			return to_string(*get<bool *>(v));
		else if (holds_alternative<int *>(v))
			return to_string(*get<int *>(v));
		else if (holds_alternative<float *>(v))
			return to_string(*get<float *>(v));
		else
			throw "Config::getValue(): Internal error";
	}

	void setValue(std::string name, std::string value)
	{
		using namespace std;
		auto v = find(name);
		stringstream ss;
		ss << value;
		if (holds_alternative<bool *>(v))
			ss >> *get<bool *>(v);
		else if (holds_alternative<int *>(v))
			ss >> *get<int *>(v);
		else if (holds_alternative<float *>(v))
			ss >> *get<float *>(v);
		else
			throw "Config::getValue(): Internal error";

		if (!ss)
			throw "Invalid type for config value " + name;
	}
};

#endif
