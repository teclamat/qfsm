#ifndef MACHINE_HPP
#define MACHINE_HPP

#include "literals.hpp"

#include <QFont>
#include <QHash>
#include <QString>
#include <QStringList>

#include <memory>

namespace qfsm {

class Project;
class State;
class Transition;
class InitialTransition;
using StatePtr = std::shared_ptr<State>;
using TransitionPtr = std::shared_ptr<Transition>;
using InitialTransitionPtr = std::shared_ptr<InitialTransition>;
using States = QHash<int, StatePtr>;

enum class ArrowType { Line, Filled, White, FilledPointed, WhitePointed };

class Machine {
 public:
  struct IoBits {
    int count;
    QStringList names;
  };

  enum class Type { Binary, Ascii, Text };

  Machine(qfsm::Project* a_project = nullptr);

  const QString& name() const { return m_name; }
  void setName(const QString& a_name) { m_name = a_name; }

  const QString& version() const { return m_version; }
  void setVersion(const QString& a_version) { m_version = a_version; }

  const QString& author() const { return m_author; }
  void setAuthor(const QString& a_author) { m_author = a_author; }

  const QString& description() const { return m_description; }
  void setDescription(const QString& a_description) { m_description = a_description; }

  QFont& stateFont() { return m_stateFont; }
  const QFont& stateFont() const { return m_stateFont; }

  QFont& transitionFont() { return m_transitionFont; }
  const QFont& transitionFont() const { return m_transitionFont; }

  const StatePtr& phantomState() const { return m_phantomState; }
  StatePtr phantomState() { return m_phantomState; }

  bool hasInitialState() const { m_initialState != nullptr; }
  const StatePtr& initialState() const { return m_initialState; }
  StatePtr initialState() { return m_initialState; }
  void setInitialState(const StatePtr& a_state) { m_initialState = a_state; }

  const InitialTransitionPtr& initialTransition() const { return m_initialTransition; }
  InitialTransitionPtr initialTransition() { return m_initialTransition; }
  void setInitialTransition(const InitialTransitionPtr& a_transition) { m_initialTransition = a_transition; }

  int mealyInputsCount() const { return m_mealyInputs.count; }
  void setMealyInputsCount(int a_count) { m_mealyInputs.count = a_count; }

  int mealyOutputsCount() const { return m_mealyOutputs.count; }
  void setMealyOutputsCount(int a_count) { m_mealyOutputs.count = a_count; }

  int mooreOutputsCount() const { return m_mooreOutputs.count; }
  void setMooreOutputsCount(int a_count) { m_mooreOutputs.count = a_count; }

  Type type() const { return m_type; }
  void setType(Type a_type) { m_type = a_type; }

  ArrowType arrowType() const { return m_arrowType; }
  void setArrowType(ArrowType a_type) { m_arrowType = a_type; }

  bool drawInitialTransition() const { return m_drawInitialTransition; }
  void setDrawInitialTransition(bool a_draw = true) { m_drawInitialTransition = a_draw; }

  void addState(const StatePtr& a_state);
  bool hasState(int a_code) const { m_states.contains(a_code); }
  const StatePtr& state(int a_code) const { return m_states.value(a_code, nullptr); }

  int getNewCode() const;

  int validStatesCount() const;

 private:
  Project* m_project;
  StatePtr m_phantomState;

  QString m_name{};
  QString m_version{};
  QString m_author{};
  QString m_description{};
  QFont m_stateFont{ u"Helvetica"_qs, 8 };
  QFont m_transitionFont{ u"Helvetica"_qs, 8 };
  StatePtr m_initialState{ nullptr };
  InitialTransitionPtr m_initialTransition{ nullptr };
  States m_states{};
  IoBits m_mealyInputs{};
  IoBits m_mealyOutputs{};
  IoBits m_mooreOutputs{};
  Type m_type{ Type::Ascii };
  ArrowType m_arrowType{ ArrowType::Filled };
  bool m_drawInitialTransition{ true };
};

} // namespace qfsm

#endif // MACHINE_HPP
