#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#if QT_VERSION >= 0x040400
#include <QAtomicInt>
#endif

template <class T, int potSize = 12> class RingBuffer {

public:
  static const int size = 1 << potSize;

  RingBuffer()
    : _writeAt(0)
    , _readAt(0)
    , _count(0)
  {}

  int writeAt() {
    return _writeAt;
  }

  void put(const T &e) {
    d[_writeAt] = e;
    _writeAt = (_writeAt + 1) & (size - 1);
#if QT_VERSION >= 0x040400
    _count.ref();
#else
    q_atomic_increment(&_count);
#endif
  }

  T &get() {
#if QT_VERSION >= 0x040400
    _count.deref();
#else
    q_atomic_decrement(&_count);
#endif
    T &r = d[_readAt];
    _readAt = (_readAt + 1) & (size - 1);
    return r;
  }

  int count() {
    return _count;
  }

protected:
  T d[size];

  int _writeAt;
  int _readAt;
#if QT_VERSION >= 0x040400
  QAtomicInt _count;
#else
  int _count;
#endif

};

#endif

