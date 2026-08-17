#pragma once
#include <string_view>
namespace neroedl::log {
void set_verbose(bool);
void info(std::string_view);
void success(std::string_view);
void error(std::string_view);
void debug(std::string_view);
} // namespace neroedl::log
