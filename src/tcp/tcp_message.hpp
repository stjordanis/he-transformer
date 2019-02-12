//*****************************************************************************
// Copyright 2018-2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#pragma once

#include <assert.h>
#include <cstring>
#include <iostream>
#include <memory>

namespace ngraph {
namespace runtime {
namespace he {
enum class MessageType {
  public_key_request,
  public_key,
  relu_request,
  relu,
  none
};

class TCPMessage {
 public:
  enum { header_length = sizeof(size_t) };
  enum { max_body_length = 1000 };

  TCPMessage()
      : m_bytes(header_length),
        m_body_length(0),
        m_type(MessageType::none),
        m_count(0) {}

  // @brief Describes TCP messages of the form: (num_bytes, MessageType, data)
  // @param count number of elements of data
  // @param size number of bytes of data in message. Must be a multiple of count
  TCPMessage(const MessageType type, const size_t count, const size_t size,
             const char* data)
      : m_type(type), m_count(count) {
    if (count != 0 && size % count != 0) {
      throw std::invalid_argument("Size must be a multiple of count");
    }

    m_bytes = header_length;
    m_bytes += sizeof(MessageType);
    m_bytes += size;

    m_body_length = m_bytes - header_length;

    std::cout << "Encoding message header" << std::endl;
    encode_header();
    encode_body();
    std::cout << "Done creating TCP message" << std::endl;
  }

  char* data() { return m_data; }

  const char* data() const { return m_data; }

  char* body() { return m_data + header_length; }

  const char* body() const { return m_data + header_length; }

  const char* msg_type_ptr() const {
    return m_data + header_length + sizeof(MessageType);
  }

  std::size_t body_length() const { return m_body_length; }

  size_t size() const { return m_bytes; }

  void encode_body() {
    // Copy message type
    std::cout << "Encoding body " << std::endl;
    std::cout << "sizeof(MessageType) " << sizeof(MessageType) << std::endl;
    std::memcpy(body(), &m_type, sizeof(MessageType));

    std::cout << "encoded body " << std::endl;
  }

  void encode_header() {
    assert(header_length == sizeof(m_bytes));

    char header[header_length + 1] = "";
    std::sprintf(header, "%4d", static_cast<int>(m_body_length));
    std::memcpy(m_data, header, header_length);

    std::cout << "Encoded header " << header << std::endl;

    assert(decode_header());
  }

  // Given m_data, parses to find m_datatype, m_count
  bool decode_body() {
    std::cout << "Decode message body " << std::endl;
    MessageType type;
    std::memcpy(&type, msg_type_ptr(), sizeof(MessageType));

    switch (type) {
      case MessageType::none:
        std::cout << "Decoded body type: none" << std::endl;
        break;

      case MessageType::public_key:
        std::cout << "Decoded body type: public_key " << std::endl;
        break;

      case MessageType::public_key_request:
        std::cout << "Decoded body type: public_key_request " << std::endl;
        break;

      default:
        std::cout << "Error decoding message type" << std::endl;
    }

    m_type = type;
  }

  bool decode_header() {
    char header[header_length + 1] = "";
    std::strncat(header, m_data, header_length);
    m_body_length = std::atoi(header);
    if (m_body_length > max_body_length) {
      m_body_length = 0;
      return false;
    }
    std::cout << "Decoding header; message size " << m_body_length << std::endl;
    return true;

    /*
    memcpy(&m_body_length, (char*)m_data_ptr, header_length);
    if (m_body_length > max_body_length) {
      std::cout << "Message body length " << m_body_length
                << " exceeds maximum " << max_body_length << std::endl;
      m_body_length = 0;
      return false;
    }
    return true; */
  }

 private:
  size_t m_bytes;        // How many bytes in the message
  size_t m_body_length;  // How many bytes in message body
  MessageType m_type;    // What data is being transmitted
  size_t m_count;        // Number of datatype in message

  char m_data[header_length + max_body_length];

  // void* m_data_ptr;
};
}  // namespace he
}  // namespace runtime
}  // namespace ngraph
