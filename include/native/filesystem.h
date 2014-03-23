#pragma once

#include <stddef.h>
#include <stdio.h>

struct fs_stream;
typedef struct fs_stream fs_stream;

fs_stream *fs_open(const char *_filename, const char *_mode);
fs_stream *fs_memopen(void *_buffer, size_t _size, const char *_mode, const char * _name);
size_t  fs_read(void *_ptr, size_t _size, size_t _count, fs_stream *_stream);
size_t  fs_write(void *_ptr, size_t _size, size_t _count, fs_stream *_stream);
int fs_seek(fs_stream *_stream, long int _offset, int _origin);
long int fs_tell(fs_stream *_stream);
int fs_close(fs_stream *_stream);

int fs_eof(fs_stream *_stream);
int fs_getc(fs_stream *_stream);
char * fs_gets(char * _str, int _num, fs_stream *_stream);
int fs_puts(const char *_str, fs_stream *_stream);

