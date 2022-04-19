#ifndef SRC_GUI_COMMON_HPP
#define SRC_GUI_COMMON_HPP

namespace qfsm::gui {
namespace zets {
constexpr double state = 1.0;
constexpr double transition = 2.0;
constexpr double controlHandle = 3.0;
} // namespace zets
namespace handle {
constexpr int start = 0;
constexpr int control1 = 1;
constexpr int control2 = 2;
constexpr int end = 3;
} // namespace handle

constexpr int STATE_TYPE = 70000;
constexpr int TRANSITION_TYPE = 80000;

} // namespace qfsm::gui

#endif // SRC_GUI_COMMON_HPP
