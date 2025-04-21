#pragma once

#include <string>

class Scanner {
public:
  Scanner(const std::string &source);

private:
  const char *current;
  int line;
};
