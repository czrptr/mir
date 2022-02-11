#pragma once

#include <vector>
#include <memory>

#include <parsing/Position.h>
// #include <parsing/Operator.h>

#define SPTR(type) using SPtr = std::shared_ptr<type>;
#define WPTR(type) using WPtr = std::weak_ptr<type>;
#define UPTR(type) using UPtr = std::unique_ptr<type>;

#define PTR(type) \
  SPTR(type) \
  WPTR(type) \
  UPTR(type)

namespace ast
{

/* ===================== Node ===================== */

struct Node
{
  PTR(Node)

private:
  Node::WPtr d_pParent;

public:
  Node(Node::SPtr pParent)
    : d_pParent(pParent)
  {}

  virtual ~Node() = default;

  virtual char const* nodeName() const = 0;
  std::string toString() const { return toString(0, {}, false); }
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const = 0;

  Node::WPtr parent() const { return d_pParent; }
  void setParent(Node::SPtr pParent) { d_pParent = pParent; }

  virtual Position start() const = 0;
  virtual Position end() const = 0;
};

/* ===================== Statement ===================== */

struct Statement : public Node
{
  PTR(Statement)

protected:
  Statement(Node::SPtr pParent)
    : Node(pParent)
  {}

public:
  virtual char const* nodeName() const override { return "Statement"; }
};

/* ===================== Root ===================== */

struct Root : public Node
{
  PTR(Root)

private:
  Position d_end;
  std::vector<Statement::SPtr> d_statements;

public:
  Root(Position end, std::vector<Statement::SPtr>&& statements)
    : Node(nullptr)
    , d_end(end)
    , d_statements(std::move(statements))
  {}

  virtual char const* nodeName() const override { return "Root"; }
  virtual Position start() const override { return Position(0, 0); }
  virtual Position end() const override { return d_end; }
  std::vector<Statement::SPtr> const& statements() const { return d_statements; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== Expression ===================== */

struct Expression : public Statement
{
  PTR(Expression)

protected:
  Expression(Node::SPtr pParent)
    : Statement(pParent)
  {}

public:
  virtual char const* nodeName() const override { return "Expression"; }
};

/* ===================== LetStatementPart ===================== */

struct LetStatementPart : public Node
{
  PTR(LetStatementPart)

private:
  Position d_start;
  std::string d_name;
  Expression::SPtr d_pValue;

public:
  LetStatementPart(Position start, std::string const& name, Expression::SPtr pValue, Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_start(start)
    , d_name(name)
    , d_pValue(pValue)
  {}

  virtual char const* nodeName() const override { return "LetStatementPart"; }
  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_pValue->end(); }
  std::string const& name() const { return d_name; }
  Expression::SPtr value() const { return d_pValue; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== LetStatement ===================== */

struct LetStatement : public Statement
{
  PTR(LetStatement)

public:
  using Part = LetStatementPart;

private:
  Position d_start;
  Position d_end;
  bool d_isPub;
  bool d_isMut;
  std::vector<Part::SPtr> d_parts;

public:
  LetStatement(Position start, Position end, bool isPub, bool isMut, std::vector<Part::SPtr>&& parts, Node::SPtr pParent = nullptr)
    : Statement(pParent)
    , d_start(start)
    , d_end(end)
    , d_isPub(isPub)
    , d_isMut(isMut)
    , d_parts(std::move(parts))
  {}

  virtual char const* nodeName() const override { return "LetStatement"; }
  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_end; }
  bool isPub() const { return d_isPub; }
  bool isMut() const { return d_isMut; }
  std::vector<Part::SPtr> const& parts() const { return d_parts; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== SymbolExpression ===================== */

struct SymbolExpression : public Expression
{
  PTR(SymbolExpression)

private:
  Position d_start;
  Position d_end;
  std::string d_name;

public:
  SymbolExpression(Position start, Position end, std::string const& name, Node::SPtr pParent = nullptr)
    : Expression(pParent)
    , d_start(start)
    , d_end(end)
    , d_name(name)
  {}

  virtual char const* nodeName() const override { return "SymbolExpression"; }
  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_end; }
  std::string const& name() const { return d_name; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

/* ===================== StringExpression ===================== */

struct StringExpression : public Expression
{
  PTR(StringExpression)

private:
  Position d_start;
  Position d_end;
  std::string d_value;

public:
  StringExpression(Position start, Position end, std::string const& value, Node::SPtr pParent = nullptr)
    : Expression(pParent)
    , d_start(start)
    , d_end(end)
    , d_value(value)
  {}

  virtual char const* nodeName() const override { return "StringExpression"; }
  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_end; }
  std::string const& value() const { return d_value; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

} // namespace ast

#undef SPTR
#undef WPTR
#undef UPTR
#undef PTR