#include "transition.hpp"

#include "state.hpp"

#include "TransitionInfo.h"

namespace qfsm {

Transition::Transition()
  : ITransition{}
  , m_startState{}
  , m_endState{}
  , m_info{ nullptr }
{
}

Transition::Transition(const StatePtr& a_startState, const StatePtr& a_endState, InfoPtr&& a_info)
  : ITransition{}
  , m_startState{ a_startState }
  , m_endState{ a_endState }
  , m_info{ std::move(a_info) }
{
}

} // namespace qfsm
