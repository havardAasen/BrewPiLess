#include "VirtualSerial.h"

QueueBuffer::QueueBuffer(const int size)
    : _buffer(new char[size])
    , _bufferSize(size)
{
}

QueueBuffer::~QueueBuffer()
{
    delete[] _buffer;
}

void QueueBuffer::print(const char c)
{
    _buffer[_writePtr] = c;

    int next = _writePtr + 1;
    if (next == _bufferSize) {
        next = 0;
    }
    _writePtr = next;

    // If buffer is full, advance read pointer (drop oldest)
    if (next == _readPtr) {
        _readPtr++;
        if (_readPtr == _bufferSize)
            _readPtr = 0;
    }
}

void QueueBuffer::print(const char *c)
{
    for (const char *cp = c; *cp != '\0'; cp++) {
        print(*cp);
    }
}

void QueueBuffer::println()
{
    print('\n');
}

int QueueBuffer::read()
{
    if (_writePtr == _readPtr) {
        return -1;
    }

    const auto r = _buffer[_readPtr];

    _readPtr++;
    if (_readPtr == _bufferSize) {
        _readPtr = 0;
    }

    return r;
}

int QueueBuffer::available() const
{
    // avoid using %(mod) which takes time;
    return (_writePtr >= _readPtr) ? (_writePtr - _readPtr) : (_bufferSize + _writePtr - _readPtr);
}
