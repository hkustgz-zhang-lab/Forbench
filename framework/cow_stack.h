#pragma once

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

namespace wasim
{

/// A stack with O(1) copy (fork) via structural sharing, for the simulator's
/// per-branch histories. Elements are held in a persistent singly-linked
/// list whose head is the top (back) of the stack:
///   - copy/move: O(1), all nodes are shared between the copies
///   - push_back / pop_back: O(1), never touch nodes shared with other stacks
///   - mutable_back(): O(1) when the top is unshared; if another stack
///     still shares the top node, that node is cloned first (copy-on-write),
///     which copies the top element -- the inherent divergence cost, paid
///     at most once per frame per fork
///   - back()/at()/for_each(): const access, O(1)/O(n-i)/O(n)
///
/// Forking a symbolic simulation branch used to deep-copy the full history
/// (O(depth x state-variables) per fork); with CowStack the fork shares all
/// frames and only genuinely diverging top frames get copied.
template <typename T>
class CowStack
{
  struct Node
  {
    T value;
    std::shared_ptr<Node> below;
  };

  std::shared_ptr<Node> head_;  // top of the stack; nullptr when empty
  size_t size_ = 0;

 public:
  CowStack() = default;
  CowStack(const CowStack &) = default;
  CowStack & operator=(const CowStack &) = default;
  CowStack(CowStack &&) = default;
  CowStack & operator=(CowStack &&) = default;

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  void clear()
  {
    head_.reset();
    size_ = 0;
  }

  template <typename... Args>
  void emplace_back(Args &&... args)
  {
    head_ = std::make_shared<Node>(
        Node{ T(std::forward<Args>(args)...), std::move(head_) });
    ++size_;
  }

  void push_back(const T & v) { emplace_back(v); }
  void push_back(T && v) { emplace_back(std::move(v)); }

  void pop_back()
  {
    assert(head_);
    head_ = std::move(head_->below);
    --size_;
  }

  /// const access to the top element
  const T & back() const
  {
    assert(head_);
    return head_->value;
  }

  /// mutable access to the top element; clones it first when shared
  T & mutable_back()
  {
    assert(head_);
    if (head_.use_count() > 1)  // another fork still holds this node
    {
      head_ = std::make_shared<Node>(
          Node{ head_->value, head_->below });
    }
    return head_->value;
  }

  /// const access by index counted from the bottom (front) of the stack,
  /// matching std::vector indexing of the old representation
  const T & at(size_t i) const
  {
    assert(i < size_);
    std::shared_ptr<Node> n = head_;
    for (size_t up = size_ - 1; up != i; --up) n = n->below;
    return n->value;
  }
  const T & operator[](size_t i) const { return at(i); }

  /// front-to-back traversal (same order as iterating the old std::vector)
  template <typename F>
  void for_each(F && f) const
  {
    // walk to the bottom, then invoke f on the way back up
    std::vector<const Node *> path;
    path.reserve(size_);
    for (const Node * n = head_.get(); n; n = n->below.get())
      path.push_back(n);
    for (auto it = path.rbegin(); it != path.rend(); ++it)
      f((*it)->value);
  }
};

}  // namespace wasim
