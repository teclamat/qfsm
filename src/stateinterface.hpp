#ifndef STATEINTERFACE_HPP
#define STATEINTERFACE_HPP

#include <QPointF>

namespace qfsm {

class IState {
public:
 virtual ~IState() = 0;
 virtual const QPointF& position() const = 0;
 virtual void setPosition(const QPointF& a_position) = 0;
 virtual int radius() const = 0;
 virtual void setRadius(int a_radius) = 0;
};
}

#endif // STATEINTERFACE_HPP
