#include "parsing/ast/LoopExpression.h"

using namespace ast;

namespace
{

struct ClauseNode final : public Node
{
  PTR(ClauseNode)

private:
  Token _else;
  Node::SPtr capture;
  BlockExpression::SPtr body;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  ClauseNode(
    Token _else,
    Node::SPtr capture,
    BlockExpression::SPtr body)
  : Node(nullptr)
  , _else(_else)
  , capture(capture)
  , body(body)
  {}

  virtual Position start() const override { return _else.start(); }
  virtual Position end() const override { return body->end(); }

  virtual bool isExpression() const override { return false; }
};

void ClauseNode::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  subNodes->reserve(2);
  if (capture != nullptr)
  {
    subNodes->push_back(capture);
  }
  subNodes->push_back(body);

  *nodeName = "Else";

  *additionalInfo = "";
}

} // anonymous namespace

void LoopExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  subNodes->reserve(4);
  subNodes->push_back(condition());
  if (hasCapture())
  {
    subNodes->push_back(capture());
  }
  subNodes->push_back(body());
  if (hasElseClause())
  {
    subNodes->push_back(std::make_shared<ClauseNode>(d_tokElse, d_pElseCapture, d_pElseBody));
  }

  *nodeName = "Loop";

  *additionalInfo = "";
}

LoopExpression::LoopExpression(
  Token loop,
  Node::SPtr pCondition,
  Node::SPtr pCapture,
  BlockExpression::SPtr pBody,
  Token _else,
  Node::SPtr pElseCapture,
  BlockExpression::SPtr pElseBody,
  Node::SPtr pParent)
: LabeledNode(pParent)
, d_tokLoop(loop)
, d_pCondition(pCondition)
, d_pCapture(pCapture)
, d_pBody(pBody)
, d_tokElse(_else)
, d_pElseCapture(pElseCapture)
, d_pElseBody(pElseBody)
{}

Position LoopExpression::end() const
{
  return hasElseClause() ? d_pElseBody->end() : d_pBody->end();
}

LoopExpression::SPtr LoopExpression::make_shared(
  Token loop,
  Node::SPtr pCondition,
  Node::SPtr pCapture,
  BlockExpression::SPtr pBody,
  Token _else,
  Node::SPtr pElseCapture,
  BlockExpression::SPtr pElseBody,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<LoopExpression>(
    loop, pCondition, pCapture, pBody, _else, pElseCapture, pElseBody, pParent);

  pCondition->setParent(pRes);
  if (pCapture != nullptr)
  {
    pCapture->setParent(pRes);
  }
  pBody->setParent(pRes);

  if (pElseCapture != nullptr)
  {
    pElseCapture->setParent(pRes);
  }
  if (pElseBody != nullptr)
  {
    pElseBody->setParent(pRes);
  }

  return pRes;
}