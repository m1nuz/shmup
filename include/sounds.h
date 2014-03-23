#pragma once

#include "wave.h"

ALuint new_sound(ALenum format, ALsizei frequency, ALsizei size, void *data);
ALuint get_sound(const char *filename);
