#pragma once
#include <chrono>
#include <cstdint>
#include <span>
#include <string>
#include <vector>
namespace neroedl::sahara {
enum class Command : std::uint32_t {
  hello = 0x1,
  hello_response = 0x2,
  read_data = 0x3,
  end_image_tx = 0x4,
  done = 0x5,
  done_response = 0x6,
  reset = 0x7,
  reset_response = 0x8,
  command_ready = 0xb,
  command_switch_mode = 0xc,
  execute = 0xd,
  execute_response = 0xe,
  execute_data = 0xf
};
struct Packet {
  Command command;
  std::uint32_t length;
  std::vector<std::uint8_t> payload;
};
struct Hello {
  std::uint32_t version, compatible, max_packet, mode;
};
Packet parse(std::span<const std::uint8_t> bytes);
Hello parse_hello(const Packet &packet);
std::vector<std::uint8_t> hello_response(const Hello &hello,
                                         std::uint32_t mode = 3);
class Transport {
public:
  virtual ~Transport() = default;
  virtual std::vector<std::uint8_t> read(std::chrono::milliseconds) = 0;
  virtual void write(std::span<const std::uint8_t>,
                     std::chrono::milliseconds) = 0;
};
struct Handshake {
  bool detected{};
  bool programmer_required{};
  std::string detail;
};
Handshake handshake(Transport &, std::chrono::milliseconds timeout);
} // namespace neroedl::sahara
