#include "parsing/ast/SwitchExpression.h"

using namespace ast;

namespace
{

struct CaseNode final : public Node
{
  PTR(CaseNode)

private:
  SwitchExpression::Case d_case;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  CaseNode(SwitchExpression::Case _case)
    : Node(nullptr)
    , d_case(_case)
  {}

  virtual Position start() const override { return d_case.value->start(); }
  virtual Position end() const override { return d_case.result->end(); }

  virtual bool isExpression() const override { return false; }
};

void CaseNode::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->size() == 0);
  subNodes->reserve(3);
  subNodes->push_back(d_case.value);
  if (d_case.capture != nullptr)
  {
    subNodes->push_back(d_case.capture);
  }
  subNodes->push_back(d_case.result);

  *nodeName = "Case";

  *additionalInfo = "";
}

} // anonymous namespace

void SwitchExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->size() == 0);
  subNodes->reserve(1 + d_cases.size());
  subNodes->push_back(d_pValue);
  for (auto const& _case : d_cases)
  {
    subNodes->push_back(std::make_shared<CaseNode>(_case));
  }

  *nodeName = "Switch";

  *additionalInfo = "";
}

SwitchExpression::SPtr SwitchExpression::make_shared(
  Token tokSwitch,
  Node::SPtr pValue,
  std::vector<Case>&& cases,
  Position end,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<SwitchExpression>(
    tokSwitch, pValue, std::move(cases), end, pParent);

  pValue->setParent(pRes);
  for (auto& _case : pRes->d_cases)
  {
    _case.value->setParent(pRes);
    if (_case.capture != nullptr)
    {
      _case.capture->setParent(pRes);
    }
    _case.result->setParent(pRes);
  }

  return pRes;
}
