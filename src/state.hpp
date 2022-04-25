#ifndef STATE_HPP
#define STATE_HPP

#include "stateinterface.hpp"

#include <QPointF>
#include <QRgb>
#include <QString>

#include <memory>

class IOInfo;

namespace qfsm {

using IoInfoPtr = std::unique_ptr<IOInfo>;

class State : public IState {
 public:
  ~State() = default;

  const QString& name() const { return m_name; }
  void setName(const QString& a_name) { m_name = a_name; }

  const QString& description() const { return m_description; }
  void setDescription(const QString& a_description) { m_description = a_description; }

  const QString& entryActions() const { return m_entryActions; }
  void setEntryActions(const QString& a_entryActions) { m_entryActions = a_entryActions; }

  const QString& exitActions() const { return m_exitActions; }
  void setExitActions(const QString& a_exitActions) { m_exitActions = a_exitActions; }

  const QPointF& position() const override { return m_position; };
  void setPosition(const QPointF& a_position) override { m_position = a_position; };

  IOInfo* mooreOutputs() { return m_mooreOutputs.get(); }
  void setMooreOutputs(IoInfoPtr&& a_mooreOutputs) { m_mooreOutputs = std::move(a_mooreOutputs); }

  QRgb color() const { return m_color; }
  void setColor(QRgb a_color) { m_color = a_color; }

  int lineWidth() const { return m_lineWidth; }
  void setLineWidth(int a_lineWidth) { m_lineWidth = a_lineWidth; }

  int code() const { return m_code; }
  void setCode(int a_code) { m_code = a_code; }

  int radius() const override { return m_radius; };
  void setRadius(int a_radius) override { m_radius = a_radius; };

  bool isFinal() const { return m_isFinal; }
  bool toggleFinal() { m_isFinal = !m_isFinal; }
  void setFinal(bool a_isFinal = true) { m_isFinal = a_isFinal; }

  bool isDeleted() const { return m_isDeleted; }

 private:
  QString m_name{};
  QString m_description{};
  QString m_entryActions{};
  QString m_exitActions{};
  QPointF m_position{};
  IoInfoPtr m_mooreOutputs{};
  QRgb m_color{};
  int m_lineWidth{};
  int m_code{};
  int m_radius{};
  bool m_isFinal{};
  bool m_isDeleted{};
};

} // namespace qfsm

#endif // STATE_HPP
