#include "scanner.h"
#include <cctype>

Scanner::Scanner(const std::string &source)
    : current_(source.c_str()), start_(source.c_str()),
      end_(source.c_str() + source.size()), line_(1) {}

Token Scanner::scanToken() {
  skipWhitespace();

  start_ = current_;

  if (isAtEnd()) {
    return makeToken(TokenType::END_OF_FILE);
  }

  char c = advance();
  if (isdigit(c)) {
    return number();
  }

  if (isalpha(c)) {
    return identifier();
  }

  switch (c) {
  case '(':
    return makeToken(TokenType::LEFT_PAREN);
  case ')':
    return makeToken(TokenType::RIGHT_PAREN);
  case '{':
    return makeToken(TokenType::LEFT_BRACE);
  case '}':
    return makeToken(TokenType::RIGHT_BRACE);
  case ',':
    return makeToken(TokenType::COMMA);
  case '.':
    return makeToken(TokenType::DOT);
  case '-':
    return makeToken(TokenType::MINUS);
  case '+':
    return makeToken(TokenType::PLUS);
  case ';':
    return makeToken(TokenType::SEMICOLON);
  case '/':
    return makeToken(TokenType::SLASH);
  case '*':
    return makeToken(TokenType::STAR);
  case '!':
    return makeToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
  case '=':
    return makeToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
  case '<':
    return makeToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
  case '>':
    return makeToken(match('=') ? TokenType::GREATER_EQUAL
                                : TokenType::GREATER);
  case '"':
    return string();
  }

  return errorToken("Unexpected character.");
}

bool Scanner::isAtEnd() const { return current_ == end_; }

Token Scanner::makeToken(TokenType type) {
  return Token{.type = type,
               .start = start_,
               .length = static_cast<int>(current_ - start_),
               .line = line_};
}

Token Scanner::errorToken(const char *message) {
  return Token{.type = TokenType::ERROR,
               .start = message,
               .length = static_cast<int>(strlen(message)),
               .line = line_};
}

char Scanner::advance() { return *current_++; }

char Scanner::peek() const { return *current_; }

char Scanner::peekNext() const {
  if (isAtEnd()) {
    return '\0';
  }
  return current_[1];
}

bool Scanner::match(char expected) {
  if (isAtEnd()) {
    return false;
  }
  if (*current_ != expected) {
    return false;
  }
  current_++;
  return true;
}

void Scanner::skipWhitespace() {
  while (!isAtEnd()) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      line_++;
      advance();
      break;
    case '/':
      if (peekNext() == '/') {
        // Skip the rest of the line
        while (!isAtEnd() && peek() != '\n') {
          advance();
        }
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

Token Scanner::string() {
  while (!isAtEnd() && peek() != '"') {
    if (peek() == '\n') {
      line_++;
    }
    advance();
  }

  if (isAtEnd()) {
    return errorToken("Unterminated string.");
  }

  // The closing "
  advance();

  return makeToken(TokenType::STRING);
}

Token Scanner::number() {
  while (isdigit(peek())) {
    advance();
  }

  if (peek() == '.' && isdigit(peekNext())) {
    advance(); // consume the '.'
    while (isdigit(peek())) {
      advance();
    }
  }

  return makeToken(TokenType::NUMBER);
}

Token Scanner::identifier() {
  while (isalpha(peek()) || isdigit(peek())) {
    advance();
  }

  return makeToken(identifierType());
}

TokenType Scanner::identifierType() const {
  switch (start_[0]) {
  case 'a':
    return checkKeyword(1, 2, "nd", TokenType::AND);
  case 'c':
    return checkKeyword(1, 4, "lass", TokenType::CLASS);
  case 'e':
    return checkKeyword(1, 3, "lse", TokenType::ELSE);
  case 'f':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'a':
        return checkKeyword(2, 3, "lse", TokenType::FALSE);
      case 'o':
        return checkKeyword(2, 1, "r", TokenType::FOR);
      case 'u':
        return checkKeyword(2, 1, "n", TokenType::FUN);
      }
    }
    break;
  case 'i':
    return checkKeyword(1, 1, "f", TokenType::IF);
  case 'n':
    return checkKeyword(1, 2, "il", TokenType::NIL);
  case 'o':
    return checkKeyword(1, 1, "r", TokenType::OR);
  case 'p':
    return checkKeyword(1, 4, "rint", TokenType::PRINT);
  case 'r':
    return checkKeyword(1, 5, "eturn", TokenType::RETURN);
  case 's':
    return checkKeyword(1, 4, "uper", TokenType::SUPER);
  case 't':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'h':
        return checkKeyword(2, 2, "is", TokenType::THIS);
      case 'r':
        return checkKeyword(2, 1, "ue", TokenType::TRUE);
      }
    }
    break;
  case 'v':
    return checkKeyword(1, 4, "ar", TokenType::VAR);
  case 'w':
    return checkKeyword(1, 5, "hile", TokenType::WHILE);
  }
  return TokenType::IDENTIFIER;
}

TokenType Scanner::checkKeyword(int start, int length, const char *rest,
                                TokenType type) const {
  if (current_ - start_ == length && memcmp(start_, rest, length) == 0) {
    return type;
  }
  return TokenType::IDENTIFIER;
}
