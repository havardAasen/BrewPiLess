#ifndef VirtualSerial_H
#define VirtualSerial_H

#include <Arduino.h>

class QueueBuffer
{
protected:
	char* _buffer;
	int _writePtr{};
	int _readPtr{};
	int  _bufferSize;
public:
	explicit QueueBuffer(int size);
	~QueueBuffer();

	void print(char c);
	void print(const char* c);
    void println(){ print('\n');}

	int read();
	int available();
};

#endif
