#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
// toolbox.h
namespace toolbox
{
  std::string to_upper_case(const std::string &input);
  std::vector<std::string> split_string(const std::string &input, char delimiter);
  std::string join_strings(const std::vector<std::string> &input, char delimiter);
  std::string trim(const std::string &input);
  std::string escape_special_characters(const std::string &input);
}

#endif // TOOLBOX_H