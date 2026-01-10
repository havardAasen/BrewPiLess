#ifndef VIRTUAL_SERIAL_H
#define VIRTUAL_SERIAL_H


class QueueBuffer {
public:
    explicit QueueBuffer(int size);
    ~QueueBuffer();
    QueueBuffer(const QueueBuffer &other) = delete;
    QueueBuffer(QueueBuffer &&other) = delete;
    QueueBuffer &operator=(const QueueBuffer &other) = delete;
    QueueBuffer &operator=(QueueBuffer &&other) = delete;

    void print(char c);
    void print(const char *c);
    void println();

    int read();

    [[nodiscard]] int available() const;

private:
    char *_buffer;
    int _writePtr{};
    int _readPtr{};
    int _bufferSize;
};

#endif
