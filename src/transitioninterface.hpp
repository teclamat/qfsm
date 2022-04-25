#ifndef TRANSITIONINTERFACE_HPP
#define TRANSITIONINTERFACE_HPP

namespace qfsm {

class ITransition {
 public:
  virtual ~ITransition() = 0;
};

} // namespace qfsm

#endif // TRANSITIONINTERFACE_HPP
