#ifndef QUEUE_BUFFER_H
#define QUEUE_BUFFER_H


/**
 * @class QueueBuffer
 * @brief A fixed-size circular buffer for storing bytes.
 *
 * QueueBuffer implements a ring buffer with separate read and write pointers.
 * Although the constructor takes a `size` parameter and allocates `size` bytes
 * of storage, the effective usable capacity is `size - 1`. One slot is always
 * kept empty to distinguish between the "buffer full" and "buffer empty"
 * states, which both otherwise produce identical pointer positions.
 *
 * Overwrite behavior:
 * - Writing always succeeds.
 * - When the buffer becomes full (i.e., only one free slot remains), the next
 *   write overwrites the oldest unread byte.
 * - On overwrite, the read pointer is advanced to maintain consistency.
 *
 * This design ensures that the buffer always contains the most recent
 * `size - 1` bytes written.
 */
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
