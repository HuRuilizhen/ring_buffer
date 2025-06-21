#include <string>

#include "ring_buffer/ring_buffer.h"

int main(int argc, char *argv[]) {
  RingBuffer::RingBuffer<std::string> buffer(3);

  buffer.push("hello");
  buffer.push("ring");
  buffer.push("buffer");

  buffer.print();

  return 0;
}
