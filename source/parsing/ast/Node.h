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

struct Node : public std::enable_shared_from_this<Node>
{
  PTR(Node)

private:
  Node::WPtr d_pParent;
  Token d_tokComptime;
  bool d_isComptime;

protected:
  Node(Node::SPtr pParent)
    : d_pParent(pParent)
    , d_isComptime(false)
  {}

  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const = 0;

public:
  virtual ~Node() = default;

  bool isComptime() const { return d_isComptime; }
  void setIsComptime(Token tokComptime);
  void setIsComptime(bool value);
  Token tokComptime() const { return d_tokComptime; }

  virtual Position start() const = 0;
  virtual Position end() const = 0;
  virtual bool isExpression() const = 0;

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

  template<typename NodeT>
  bool is() const
  {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnonnull-compare"

    return dynamic_cast<NodeT const*>(this) != nullptr;

    #pragma GCC diagnostic pop
  }

  template<typename NodeT>
  std::shared_ptr<NodeT> as()
  {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnonnull-compare"

    if (this == nullptr)
    {
      return nullptr;
    }

    #pragma GCC diagnostic pop

    auto pRes = std::dynamic_pointer_cast<NodeT>(shared_from_this());
    assert(pRes != nullptr);
    return pRes;
  }

  std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const;
  std::string toString() const { return toString(0, {}, true); }
};

} // namespace ast