#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define WINDOW_TITLE "Particles"

struct Config
{
	int width = 768, height = 768;
	bool enableVSync = true;
	int particleCountX = 1024;
	int particleCountY = 1024;

	float forceMultiplier = 10.0;

	float colorRedMul = 0.1;
	float colorRedAdd = -1.0;
	float colorGreenMul = 0.1;
	float colorGreenAdd = 0.0;
	float colorBlueMul = 0.1;
	float colorBlueAdd = -1.0;

	float massMin = 0.75;
	float massMax = 1.25;

	float minimumDistance = 50.0;
};

#endif
