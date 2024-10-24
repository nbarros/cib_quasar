#include "toolbox.h"

namespace toolbox
{

  // Converts a string to uppercase
  std::string to_upper_case(const std::string &input)
  {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
  }

  // Splits a string by a delimiter and returns a vector of substrings
  std::vector<std::string> split_string(const std::string &input, char delimiter)
  {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, delimiter))
    {
      result.push_back(item);
    }
    return result;
  }

  // Joins a vector of strings into a single string with a delimiter
  std::string join_strings(const std::vector<std::string> &input, char delimiter)
  {
    std::ostringstream oss;
    for (size_t i = 0; i < input.size(); ++i)
    {
      if (i != 0)
      {
        oss << delimiter;
      }
      oss << input[i];
    }
    return oss.str();
  }

  // Trims whitespace from both ends of a string
  std::string trim(const std::string &input)
  {
    auto start = input.find_first_not_of(" \t\n\r\f\v");
    auto end = input.find_last_not_of(" \t\n\r\f\v");
    return (start == std::string::npos || end == std::string::npos) ? "" : input.substr(start, end - start + 1);
  }
  // Escapes special characters in a string
  std::string escape_special_characters(const std::string &input)
  {
    std::string result;
    for (char c : input)
    {
      switch (c)
      {
      case '\n':
        result += "\\n";
        break;
      case '\t':
        result += "\\t";
        break;
      case '\r':
        result += "\\r";
        break;
      case '\f':
        result += "\\f";
        break;
      case '\v':
        result += "\\v";
        break;
      default:
        result += c;
        break;
      }
    }
    return result;
  }
} // namespace toolbox
