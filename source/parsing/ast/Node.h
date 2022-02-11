#pragma once

#include <parsing/Position.h>
#include <parsing/Operator.h>

#include <cassert>
#include <vector>
#include <memory>

#define SPTR(type) using SPtr = std::shared_ptr<type>;
#define WPTR(type) using WPtr = std::weak_ptr<type>;
#define UPTR(type) using UPtr = std::unique_ptr<type>;

#define PTR(type) \
  SPTR(type) \
  WPTR(type) \
  UPTR(type)

namespace ast
{

struct Node
{
  PTR(Node)

private:
  Node::WPtr d_pParent;

protected:
  Node(Node::SPtr pParent)
    : d_pParent(pParent)
  {}

public:
  virtual ~Node() = default;

  virtual Position start() const = 0;
  virtual Position end() const = 0;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const = 0;
  virtual bool canBeUsedAsExpression() const = 0;

  std::string toString() const { return toString(0, {}, false); }
  void setParent(Node::SPtr pParent) { d_pParent = pParent; }

  template<typename NodeT = Node>
  std::weak_ptr<NodeT> parent() const
  {
    if (auto pSharedParent = d_pParent.lock(); pSharedParent != nullptr)
    {
      auto pParentAsNodeT = std::dynamic_pointer_cast<NodeT>(pSharedParent);
      assert(pParentAsNodeT != nullptr);
      return pParentAsNodeT;
    }
    return std::weak_ptr<NodeT>();
  }
};

} // namespace ast