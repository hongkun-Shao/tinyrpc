#ifndef TINYRPC_NET_TCP_TCP_BUFFER_H
#define TINYRPC_NET_TCP_TCP_BUFFER_H

#include <vector>

namespace tinyrpc {
class TcpBuffer {
 public:
  typedef std::shared_ptr<TcpBuffer> s_ptr;
  TcpBuffer(int size);
  ~TcpBuffer();

  // get the size of unread buffer
  int GetReadSize();
  // get the size of unwrite buffer
  int GetWriteSize();

  int get_read_index() { return m_read_index_; }

  int get_write_index() { return m_write_index_; }

  void WriteToBuffer(const char* buf, int size);

  void ReadFromBuffer(std::vector<char>& res, int size);

  void ResizeBuffer(int new_size);

  // auto adjust buffer when m_read_index > 1/3 m_size_
  void AdjustBuffer();

  void MoveReadIndex(int offset);

  void MoveWriteIndex(int offset);

 private:
  int m_read_index_{0};
  int m_write_index_{0};
  int m_size_{0};

 public:
  std::vector<char> m_buffer_;
};

}  // namespace tinyrpc

#endif