#include "textures.h"

GLuint
new_texture2D(GLint internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width , height, 0, format, type, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

GLuint
new_sampler2D(GLint mag_filter, GLint min_filter, GLint wrap)
{
    GLuint sampler;
    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, wrap);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, wrap);

    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, mag_filter);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, min_filter);

    return sampler;
}

GLuint
get_texture(const char *filename)
{
    GLuint iformat;
    GLenum format;
    GLsizei width;
    GLsizei height;

    void *data = load_targa(filename, &iformat, &format, &width, &height);

    if(!data)
        return 0;

    GLuint tex = new_texture2D(iformat, width, height, format, GL_UNSIGNED_BYTE, data);

    free(data);

    return tex;
}
