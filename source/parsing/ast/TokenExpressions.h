// #pragma once

#include <parsing/ast/Node.h>
#include <parsing/Token.h>

namespace ast
{

/* ===================== TokenExpression ===================== */

struct TokenExpression : public Node
{
  PTR(TokenExpression)

protected:
  Token d_token;

protected:
  TokenExpression(Token token, Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_token(token)
  {}

public:
  virtual Position start() const override { return d_token.start(); }
  virtual Position end() const override { return d_token.end(); }
  virtual bool isExpression() const override { return true; }
};

/* ===================== SymbolExpression ===================== */

struct SymbolExpression final : public TokenExpression
{
  PTR(SymbolExpression)

public:
  SymbolExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {}

  std::string_view name() const { return d_token.text(); }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== BuiltinExpression ===================== */

struct BuiltinExpression final : public TokenExpression
{
  PTR(BuiltinExpression)

public:
  BuiltinExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {
    assert(d_token.text()[0] == '@');
  }

  std::string_view name() const { return d_token.text().substr(1); }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== StringExpression ===================== */

struct StringExpression final : public TokenExpression
{
  PTR(StringExpression)

public:
  StringExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {}

  std::string_view value() const
  {
    auto const text = d_token.text();
    return text.substr(1, text.length() - 2);
  }
  std::string_view quotedValue() const { return d_token.text(); }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== NumberExpression ===================== */

struct NumberExpression final : public TokenExpression
{
  PTR(NumberExpression)

public:
  NumberExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {}

  std::string_view valueToString() const { return d_token.text(); }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== BoolExpression ===================== */

struct BoolExpression final : public TokenExpression
{
  PTR(BoolExpression)

private:
  bool d_value;

public:
  BoolExpression(Token token, bool value, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
    , d_value(value)
  {
    assert(token.text() == "true" || token.text() == "false");
  }

  bool value() const { return d_value; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== NullExpression ===================== */

struct NullExpression final : public TokenExpression
{
  PTR(NullExpression)

public:
  NullExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {
    assert(token.text() == "null");
  }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== UndefinedExpression ===================== */

struct UndefinedExpression final : public TokenExpression
{
  PTR(UndefinedExpression)

public:
  UndefinedExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {
    assert(token.text() == "undefined");
  }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== UnreachableExpression ===================== */

struct UnreachableExpression final : public TokenExpression
{
  PTR(UnreachableExpression)

public:
  UnreachableExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {
    assert(token.text() == "unreachable");
  }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

} // namespace ast