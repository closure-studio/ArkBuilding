#include "external/byte_array_buffer.h"

#include <cassert>


byte_array_buffer::byte_array_buffer(const uint8_t *begin, const size_t size) :
begin_(begin),
end_(begin + size),
current_(begin_)
{
    assert(std::less_equal<const uint8_t *>()(begin_, end_));
}

byte_array_buffer::byte_array_buffer(const char* begin, const size_t size) :
begin_(reinterpret_cast<const uint8_t *>(begin)),
end_(reinterpret_cast<const uint8_t *>(begin) + size),
current_(begin_)
{
    static_assert(sizeof(char) == sizeof(uint8_t), "char and uint8_t must be the same size");
    assert(std::less_equal<const uint8_t *>()(begin_, end_));
}

byte_array_buffer::int_type byte_array_buffer::underflow()
{
    if (current_ == end_)
        return traits_type::eof();

    return traits_type::to_int_type(*current_);
}

byte_array_buffer::int_type byte_array_buffer::uflow()
{
    if (current_ == end_)
        return traits_type::eof();

    return traits_type::to_int_type(*current_++);
}

byte_array_buffer::int_type byte_array_buffer::pbackfail(int_type ch)
{
    if (current_ == begin_ || (ch != traits_type::eof() && ch != current_[-1]))
        return traits_type::eof();

    return traits_type::to_int_type(*--current_);
}

std::streamsize byte_array_buffer::showmanyc()
{
    assert(std::less_equal<const uint8_t *>()(current_, end_));
    return end_ - current_;
}


std::streampos byte_array_buffer::seekoff ( std::streamoff off, std::ios_base::seekdir way,
                                           std::ios_base::openmode which )
{
    if (way == std::ios_base::beg)
    {
        current_ = begin_ + off;
    }
    else if (way == std::ios_base::cur)
    {
        current_ += off;
    }
    else if (way == std::ios_base::end)
    {
        current_ = end_ + off;
    }

    if (current_ < begin_ || current_ > end_)
        return -1;


    return current_ - begin_;
}

std::streampos byte_array_buffer::seekpos ( std::streampos sp,
                                           std::ios_base::openmode which )
{
    current_ = begin_ + sp;

    if (current_ < begin_ || current_ > end_)
        return -1;

    return current_ - begin_;
}