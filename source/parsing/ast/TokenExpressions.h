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

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  SymbolExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {}

  std::string_view name() const { return d_token.text(); }
};

/* ===================== BuiltinExpression ===================== */

struct BuiltinExpression final : public TokenExpression
{
  PTR(BuiltinExpression)

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  BuiltinExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {
    assert(d_token.text()[0] == '@');
  }

  std::string_view name() const { return d_token.text().substr(1); }
};

/* ===================== StringExpression ===================== */

struct StringExpression final : public TokenExpression
{
  PTR(StringExpression)

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  StringExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {}

  std::string_view value() const;
  std::string_view quotedValue() const { return d_token.text(); }
};

/* ===================== NumberExpression ===================== */

struct NumberExpression final : public TokenExpression
{
  PTR(NumberExpression)

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  NumberExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {}

  std::string_view valueToString() const { return d_token.text(); }
};

/* ===================== BoolExpression ===================== */

struct BoolExpression final : public TokenExpression
{
  PTR(BoolExpression)

private:
  bool d_value;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  BoolExpression(Token token, bool value, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
    , d_value(value)
  {
    assert(token.text() == "true" || token.text() == "false");
  }

  bool value() const { return d_value; }
};

/* ===================== NullExpression ===================== */

struct NullExpression final : public TokenExpression
{
  PTR(NullExpression)

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  NullExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {
    assert(token.text() == "null");
  }
};

/* ===================== UndefinedExpression ===================== */

struct UndefinedExpression final : public TokenExpression
{
  PTR(UndefinedExpression)

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  UndefinedExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {
    assert(token.text() == "undefined");
  }
};

/* ===================== UnreachableExpression ===================== */

struct UnreachableExpression final : public TokenExpression
{
  PTR(UnreachableExpression)

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  UnreachableExpression(Token token, Node::SPtr pParent = nullptr)
    : TokenExpression(token, pParent)
  {
    assert(token.text() == "unreachable");
  }
};

} // namespace ast