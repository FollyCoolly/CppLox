#include "scanner.h"

Scanner::Scanner(const std::string &source)
    : current(source.c_str()), line(1) {}
