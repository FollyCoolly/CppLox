#pragma once

#include <cstring>
#include <string>

enum class TokenType {
  // Single-character tokens
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,

  // One or two character tokens
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,

  // Literals
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords
  AND,
  CLASS,
  ELSE,
  FALSE,
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,

  // Other
  ERROR,
  END_OF_FILE
};

struct Token {
  TokenType type;
  const char *start;
  int length;
  int line;

  bool operator==(const Token &other) const {
    if (length != other.length)
      return false;
    return memcmp(start, other.start, length);
  }

  static Token emptyToken() { return Token{.start = "", .length = 0}; }
  static Token thisToken() { return Token{.start = "this", .length = 4}; }
};

class Scanner {
public:
  Scanner(const std::string &source);
  Token scanToken();

private:
  bool isAtEnd() const;
  Token makeToken(TokenType type);
  Token errorToken(const char *message);
  char advance();
  char peek() const;
  char peekNext() const;
  bool match(char expected);
  void skipWhitespace();
  Token string();
  Token number();
  Token identifier();
  TokenType identifierType() const;
  TokenType checkKeyword(int start, int length, const char *rest,
                         TokenType type) const;

  const char *current_;
  const char *start_;
  const char *end_;
  int line_;
};
