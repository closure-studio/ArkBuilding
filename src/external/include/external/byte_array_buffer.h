#pragma once

#include <cstdio>
#include <string>
#include <list>
#include <fstream>
#include <iostream>

//
// http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
//

class byte_array_buffer : public std::streambuf
{
public:
    byte_array_buffer(const uint8_t *begin, const size_t size);
    byte_array_buffer(const char *begin, const size_t size);

private:
    int_type underflow();
    int_type uflow();
    int_type pbackfail(int_type ch);
    std::streamsize showmanyc();
    std::streampos seekoff ( std::streamoff off, std::ios_base::seekdir way,
                            std::ios_base::openmode which = std::ios_base::in | std::ios_base::out );
    std::streampos seekpos ( std::streampos sp,
                            std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

    // copy ctor and assignment not implemented;
    // copying not allowed
    byte_array_buffer(const byte_array_buffer &);
    byte_array_buffer &operator= (const byte_array_buffer &);

private:
    const uint8_t * const begin_;
    const uint8_t * const end_;
    const uint8_t * current_;
};