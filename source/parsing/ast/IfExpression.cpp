#include "parsing/ast/IfExpression.h"

using namespace ast;

struct ClauseNode final : public Node
{
  PTR(ClauseNode)

private:
  IfExpression::Clause d_caluse;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  ClauseNode(IfExpression::Clause clause)
    : Node(nullptr)
    , d_caluse(clause)
  {}

  virtual Position start() const override { return d_caluse.tokStart.start(); }
  virtual Position end() const override { return d_caluse.block->end(); }

  virtual bool isExpression() const override { return false; }
};

void ClauseNode::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  subNodes->reserve(3);
  if (d_caluse.tag == IfExpression::Clause::ElseIf)
  {
    subNodes->push_back(d_caluse.condition);
    if (d_caluse.capture != nullptr)
    {
      subNodes->push_back(d_caluse.capture);
    }
    subNodes->push_back(d_caluse.block);

   *nodeName = "ElseIf";
  }
  else // Else
  {
    if (d_caluse.capture != nullptr)
    {
      subNodes->push_back(d_caluse.capture);
    }

   *nodeName = "Else";
  }

  *additionalInfo = "";
}

Position IfExpression::start() const
{
  return d_clauses.front().tokStart.start();
}

Position IfExpression::end() const
{
  return clauses().back().block->end();
}

void IfExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->size() == 0);
  subNodes->reserve(2 * (1 + clauses().size()));

  auto const& ifClause = clauses().front();
  subNodes->push_back(ifClause.condition);
  if (ifClause.capture != nullptr)
  {
    subNodes->push_back(ifClause.capture);
  }

  for (size_t i = 1; i < clauses().size(); i += 1)
  {
    subNodes->push_back(std::make_shared<ClauseNode>(clauses()[i]));
  }

  *nodeName = "If";

  // TODO (bool ?T or E!T)
  *additionalInfo = "";
}

IfExpression::SPtr IfExpression::make_shared(
  std::vector<Clause>&& clauses,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<IfExpression>(std::move(clauses), pParent);
  for (auto& clause : pRes->d_clauses)
  {
    if (clause.condition != nullptr)
    {
      clause.condition->setParent(pRes);
    }
    if (clause.capture != nullptr)
    {
      clause.capture->setParent(pRes);
    }
  }
  return pRes;
}