
#include "tinyrpc/net/tcp/tcp_buffer.h"

#include <string.h>

#include <algorithm>

#include "tinyrpc/tool/log.h"

namespace tinyrpc {
TcpBuffer::TcpBuffer(int size) : m_size_(size) { m_buffer_.resize(size); }

TcpBuffer::~TcpBuffer() {}

// get the size of unread buffer
int TcpBuffer::GetReadSize() { return m_write_index_ - m_read_index_; }

// get the size of unwrite buffer
int TcpBuffer::GetWriteSize() { return m_buffer_.size() - m_write_index_; }

void TcpBuffer::WriteToBuffer(const char* buf, int size) {
  if (size > GetWriteSize()) {
    // adjust buffer size, add memry
    int new_size = (int)(1.5 * (m_write_index_ + size));
    ResizeBuffer(new_size);
  }
  memcpy(&m_buffer_[m_write_index_], buf, size);
  m_write_index_ += size;
}

void TcpBuffer::ReadFromBuffer(std::vector<char>& res, int size) {
  if (GetReadSize() == 0) {
    return;
  }

  int read_size = std::min(size, GetReadSize());

  std::vector<char> temp(read_size);
  memcpy(&temp[0], &m_buffer_[m_read_index_], read_size);

  res.swap(temp);
  m_read_index_ += read_size;

  AdjustBuffer();
}

void TcpBuffer::ResizeBuffer(int new_size) {
  std::vector<char> temp(new_size);

  // avoid segment falut
  int count = std::min(new_size, GetReadSize());

  memcpy(&temp[0], &m_buffer_[m_read_index_], count);
  m_buffer_.swap(temp);

  m_read_index_ = 0;
  m_write_index_ = m_read_index_ + count;
}

void TcpBuffer::AdjustBuffer() {
  if (m_read_index_ < int(m_buffer_.size() / 3)) {
    return;
  }
  std::vector<char> buffer(m_buffer_.size());
  int count = GetReadSize();

  memcpy(&buffer[0], &m_buffer_[m_read_index_], count);
  m_buffer_.swap(buffer);
  m_read_index_ = 0;
  m_write_index_ = m_read_index_ + count;

  buffer.clear();
}

void TcpBuffer::MoveReadIndex(int offset) {
  size_t j = m_read_index_ + offset;
  if (j >= m_buffer_.size()) {
    ERRORLOG(
        "moveReadIndex error, invalid offset %d, old_read_index %d, buffer "
        "size %d",
        offset, m_read_index_, m_buffer_.size());
    return;
  }
  m_read_index_ = j;
  AdjustBuffer();
}

void TcpBuffer::MoveWriteIndex(int offset) {
  size_t j = m_write_index_ + offset;
  if (j >= m_buffer_.size()) {
    ERRORLOG(
        "moveWriteIndex error, invalid offset %d, old_read_index %d, buffer "
        "size %d",
        offset, m_read_index_, m_buffer_.size());
    return;
  }
  m_write_index_ = j;
  AdjustBuffer();
}

}  // namespace tinyrpc