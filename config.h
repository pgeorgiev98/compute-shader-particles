#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define WINDOW_TITLE "Particles"

struct Config
{
	int width = 512, height = 512;
	bool enableVSync = true;
};

#endif
