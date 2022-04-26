#include "state.hpp"

#include "transition.hpp"

namespace qfsm {

void State::appendStartTransition(const TransitionPtr& a_transition)
{
  m_startTransitions.push_back(a_transition);

  StatePtr endState = a_transition->endState();
  if (endState) {
    endState->appendEndTransition(a_transition);
  }
}

} // namespace qfsm
