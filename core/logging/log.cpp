#include "neroedl/log.hpp"
#include <iostream>
namespace neroedl::log {
namespace {
bool verbose{};
}
void set_verbose(bool v) { verbose = v; }
void info(std::string_view s) { std::cout << "[*] " << s << '\n'; }
void success(std::string_view s) { std::cout << "[+] " << s << '\n'; }
void error(std::string_view s) { std::cerr << "[-] " << s << '\n'; }
void debug(std::string_view s) {
  if (verbose)
    std::cout << "[.] " << s << '\n';
}
} // namespace neroedl::log
