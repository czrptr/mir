#include "parsing/ast/ContinueStatement.h"

#include <fmt/core.h>
#include <fmt/color.h>

using namespace ast;

void ContinueStatement::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());

  *nodeName = "Continue";

  *additionalInfo  = isLabeled() ? fmt::format(fmt::emphasis::italic, "{}", label().text()) : "";
}

Position ContinueStatement::end() const
{
  if (isLabeled())
  {
    return label().end();
  }
  return d_tokContinue.end();
}