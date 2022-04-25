#ifndef TRANSITION_HPP
#define TRANSITION_HPP

#include "transitioninterface.hpp"

#include <QPointF>
#include <QString>

#include <memory>

class TransitionInfo;

namespace qfsm {

class State;
using StatePtr = std::shared_ptr<State>;
using InfoPtr = std::unique_ptr<TransitionInfo>;

class Transition : public ITransition {
 public:
  Transition();
  Transition(const StatePtr& a_stateStart, const StatePtr& a_stateEnd, InfoPtr&& a_info);
  ~Transition() = default;

  bool hasStartState() const { return m_startState != nullptr; }
  void setStartState(const StatePtr& a_state) { m_startState = a_state; }

  void setEndState(const StatePtr& a_state) { m_endState = a_state; }

  const QString& description() const { return m_description; }
  void setDescription(const QString& a_description) { m_description = a_description; }

  const QPointF& startPosition() const { return m_startPosition; }
  void setStartPosition(const QPointF& a_position) { m_startPosition = a_position; }

  const QPointF& endPosition() const { return m_endPosition; }
  void setEndPosition(const QPointF& a_position) { m_endPosition = a_position; }

  const QPointF& controlPoint1() const { return m_controlPoint1; }
  void setControlPoint1(const QPointF& a_position) { m_controlPoint1 = a_position; }

  const QPointF& controlPoint2() const { return m_controlPoint2; }
  void setControlPoint2(const QPointF& a_position) { m_controlPoint2 = a_position; }

  bool isStraight() const { return m_isStraight; }
  void setStraight(bool a_isStraight = true) { m_isStraight = a_isStraight; }

 private:
  StatePtr m_startState;
  StatePtr m_endState;
  InfoPtr m_info;

  QString m_description{};
  QPointF m_startPosition{};
  QPointF m_endPosition{};
  QPointF m_controlPoint1{};
  QPointF m_controlPoint2{};
  bool m_isStraight{};
  bool m_isDeleted{};
};

} // namespace qfsm

#endif // TRANSITION_HPP
