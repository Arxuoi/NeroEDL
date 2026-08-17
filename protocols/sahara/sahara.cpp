#include "neroedl/sahara.hpp"
#include <stdexcept>
namespace neroedl::sahara {
namespace {
std::uint32_t u32(std::span<const std::uint8_t> b, size_t p) {
  if (p + 4 > b.size())
    throw std::invalid_argument("truncated Sahara field");
  return static_cast<uint32_t>(b[p]) | (static_cast<uint32_t>(b[p + 1]) << 8) |
         (static_cast<uint32_t>(b[p + 2]) << 16) |
         (static_cast<uint32_t>(b[p + 3]) << 24);
}
void put(std::vector<uint8_t> &b, uint32_t v) {
  for (int i = 0; i < 4; ++i)
    b.push_back(static_cast<uint8_t>(v >> (i * 8)));
}
} // namespace
Packet parse(std::span<const uint8_t> b) {
  if (b.size() < 8)
    throw std::invalid_argument("Sahara packet shorter than header");
  auto length = u32(b, 4);
  if (length < 8 || length > 1024 * 1024)
    throw std::invalid_argument("invalid Sahara packet length");
  if (length != b.size())
    throw std::invalid_argument("Sahara packet length mismatch");
  return {static_cast<Command>(u32(b, 0)), length, {b.begin() + 8, b.end()}};
}
Hello parse_hello(const Packet &p) {
  if (p.command != Command::hello || p.length < 0x30 || p.payload.size() < 16)
    throw std::invalid_argument("not a complete Sahara HELLO");
  return {u32(p.payload, 0), u32(p.payload, 4), u32(p.payload, 8),
          u32(p.payload, 12)};
}
std::vector<uint8_t> hello_response(const Hello &h, uint32_t mode) {
  std::vector<uint8_t> b;
  put(b, static_cast<uint32_t>(Command::hello_response));
  put(b, 0x30);
  put(b, h.version);
  put(b, h.compatible);
  put(b, 0);
  put(b, mode);
  while (b.size() < 0x30)
    put(b, 0);
  return b;
}
Handshake handshake(Transport &t, std::chrono::milliseconds timeout) {
  try {
    auto p = parse(t.read(timeout));
    auto h = parse_hello(p);
    t.write(hello_response(h), timeout);
    return {true, h.mode == 0,
            "Sahara HELLO accepted; programmer transfer is not performed"};
  } catch (const std::exception &e) {
    return {false, false, e.what()};
  }
}
} // namespace neroedl::sahara
