#include "machine.hpp"

#include "state.hpp"

#include <algorithm>

namespace qfsm {

Machine::Machine(qfsm::Project* a_project)
  : m_project{ a_project }
  , m_phantomState{ std::make_unique<State>() }
{
  m_stateFont.setStyleHint(QFont::SansSerif);
  m_transitionFont.setStyleHint(QFont::SansSerif);
}

int Machine::getNewCode() const
{
  int code = 0;
  while (m_states.contains(code)) {
    ++code;
  }
  return code;
}

void Machine::addState(const StatePtr& a_state)
{
  if (a_state == nullptr) {
    return;
  }

  m_states.insert(a_state->code(), a_state);

  if (validStatesCount() == 1) {
    m_initialState = a_state;
    m_initialTransition = std::make_shared<InitialTransition>(m_phantomState, a_state);
  }
}

int Machine::validStatesCount() const
{
  return static_cast<int>(
      std::count_if(m_states.cbegin(), m_states.cend(), [](const StatePtr& a_state) { return !a_state->isDeleted(); }));
}

} // namespace qfsm
