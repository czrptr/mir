#pragma once

#include <parsing/ast/Node.h>
#include <parsing/Token.h>

namespace ast
{

struct LetStatementPart final : public Node
{
  PTR(LetStatementPart)

private:
  Position d_start;
  std::string_view d_name;
  Node::SPtr d_pType;
  Node::SPtr d_pValue;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  LetStatementPart(
    Position start,
    std::string_view name,
    Node::SPtr pType,
    Node::SPtr pValue,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_start(start)
    , d_name(name)
    , d_pType(pType)
    , d_pValue(pValue)
  {
    assert(pValue != nullptr);
  }

  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_pValue->end(); }
  virtual bool isExpression() const override { return false; }

  std::string_view name() const { return d_name; }
  bool hasType() const { return d_pType != nullptr; }
  Node::SPtr type() const { return d_pType; }
  Node::SPtr value() const { return d_pValue; }

  static LetStatementPart::SPtr make_shared(
    Position start,
    std::string_view name,
    Node::SPtr pType,
    Node::SPtr pValue,
    Node::SPtr pParent = nullptr);
};

struct LetStatement final : public Node
{
  PTR(LetStatement)

public:
  using Part = LetStatementPart;

private:
  Position d_start;
  bool d_isPub;
  bool d_isMut;
  std::vector<Part::SPtr> d_parts;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  LetStatement(
    Position start,
    bool isPub,
    bool isMut,
    std::vector<Part::SPtr>&& parts,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_start(start)
    , d_isPub(isPub)
    , d_isMut(isMut)
    , d_parts(std::move(parts))
  {}

  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_parts.back()->end(); }
  virtual bool isExpression() const override { return false; }

  bool isPub() const { return d_isPub; }
  bool isMut() const { return d_isMut; }
  std::vector<Part::SPtr> const& parts() const { return d_parts; }

  static LetStatement::SPtr make_shared(
    Position start,
    bool isPub,
    bool isMut,
    std::vector<Part::SPtr>&& parts,
    Node::SPtr pParent = nullptr);
};

} // namespace ast