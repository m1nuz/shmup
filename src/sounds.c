#include "sounds.h"

#include <stdlib.h>

ALuint
new_sound(ALenum format, ALsizei frequency, ALsizei size, void *data)
{
    ALuint buf;
    alGenBuffers(1, &buf);
    alBufferData(buf, format, data, size, frequency);

    return buf;
}

ALuint
get_sound(const char *filename)
{
    ALenum format;
    ALsizei frequency;
    ALsizei size;

    void *data = load_wave(filename, &format, &frequency, &size);

    if(!data)
        return 0;

    ALuint buf = new_sound(format, frequency, size, data);

    free(data);

    return buf;
}
