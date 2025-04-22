#include "scanner.h"

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
