#ifndef INITIALTRANSITION_HPP
#define INITIALTRANSITION_HPP

#include "transitioninterface.hpp"

#include <QPointF>

#include <memory>

namespace qfsm {

class State;
using StatePtr = std::shared_ptr<State>;

class InitialTransition : public ITransition {
 public:
  ~InitialTransition() = default;

  void setStartState(const StatePtr& a_state) { m_startState = a_state; }
  void setEndState(const StatePtr& a_state) { m_endState = a_state; }
  void setStartPosition(const QPointF& a_position) { m_startPosition = a_position; }
  void setEndPosition(const QPointF& a_position) { m_endPosition = a_position; }

 private:
  StatePtr m_startState{};
  StatePtr m_endState{};
  QPointF m_startPosition{};
  QPointF m_endPosition{};
};

} // namespace qfsm

#endif // INITIALTRANSITION_HPP
