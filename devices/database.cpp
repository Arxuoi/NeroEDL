#include "neroedl/database.hpp"
#include <fstream>
#include <regex>
#include <stdexcept>
namespace neroedl {
namespace {
std::string field(const std::string &s, const char *n) {
  std::regex r(std::string("\\\"") + n + "\\\"\\s*:\\s*\\\"([^\\\"]*)\\\"");
  std::smatch m;
  if (!std::regex_search(s, m, r))
    throw std::invalid_argument(std::string("missing database field: ") + n);
  return m[1];
}
std::vector<std::string> array(const std::string &s, const char *n) {
  std::regex r(std::string("\\\"") + n + "\\\"\\s*:\\s*\\[([\\s\\S]*?)\\]");
  std::smatch m;
  if (!std::regex_search(s, m, r))
    throw std::invalid_argument(std::string("missing database array: ") + n);
  std::vector<std::string> v;
  std::regex q("\\\"([^\\\"]*)\\\"");
  for (std::sregex_iterator i(m[1].first, m[1].second, q), e; i != e; ++i)
    v.push_back((*i)[1]);
  return v;
}
} // namespace
DeviceDatabase DeviceDatabase::load(const std::string &path) {
  std::ifstream f(path);
  if (!f)
    throw std::runtime_error("cannot open device database: " + path);
  std::string s((std::istreambuf_iterator<char>(f)), {});
  DeviceDatabase db;
  std::regex object("\\{[^{}]*\\}");
  for (std::sregex_iterator i(s.begin(), s.end(), object), e; i != e; ++i) {
    auto o = i->str();
    auto status = field(o, "status");
    if (status != "TESTED" && status != "COMMUNITY_TESTED" &&
        status != "EXPERIMENTAL" && status != "UNTESTED" &&
        status != "UNSUPPORTED")
      throw std::invalid_argument("invalid compatibility status");
    db.records_.push_back({field(o, "vendor"), field(o, "model"),
                           field(o, "soc"), status, array(o, "usb_ids"),
                           array(o, "entry_methods")});
  }
  if (db.records_.empty())
    throw std::invalid_argument("device database contains no records");
  return db;
}
} // namespace neroedl
