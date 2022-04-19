#include "optionsmanager.hpp"

#include "MainWindow.h"
#include "gui/actionsmanager.hpp"
#include "gui/view.hpp"

#include <QColor>
#include <QMainWindow>
#include <QSettings>
#include <QStringList>

namespace qfsm {

constexpr int MAX_RECENTS_ENTRIES = 10;

QHash<OptionsManager::Group, QString> GROUP_NAME{ { option::Group::General, QStringLiteral("general") },
                                                  { option::Group::View, QStringLiteral("view") },
                                                  { option::Group::Print, QStringLiteral("print") } };

OptionsManager::OptionsManager(QMainWindow* a_window)
  : QObject{ a_window }
  , m_window{ qobject_cast<MainWindow*>(a_window) }
  , m_settings{ new QSettings{ this } }
{
  readOption(Group::View, option::stateEncoding, false, { Affects::View, Affects::Action });
  readOption(Group::View, option::mooreOutputs, true, { Affects::View, Affects::Action });
  readOption(Group::View, option::mealyInputs, false, { Affects::View, Affects::Action });
  readOption(Group::View, option::mealyOutputs, false, { Affects::View, Affects::Action });
  readOption(Group::View, option::grid, false, { Affects::View, Affects::Action });
  readOption(Group::View, option::gridSize, 10, Affects::View);
  readOption(Group::View, option::gridColor, QColor{ 200, 200, 200 }.rgb(), Affects::View);
  readOption(Group::View, option::shadows, true, { Affects::View, Affects::Action });
  readOption(Group::View, option::shadowColor, QColor{ Qt::darkGray }.rgb(), Affects::View);
  readOption(Group::View, option::ioView, false, Affects::Nothing);

  m_recentsList = m_settings->value(settingKey(Group::General, option::recentFiles), QStringList{}).toStringList();
}

const QVariant OptionsManager::variant(Group a_group, const QString& a_name) const
{
  const QString key = settingKey(a_group, a_name);

  if (m_optionsMap.contains(key)) {
    return m_optionsMap[key].value;
  }

  qWarning() << "[OptionsManager] Unknown option requested:" << key;
  return {};
}

bool OptionsManager::toggleValue(Group a_group, const QString& a_name)
{
  const QVariant optionValue = variant(a_group, a_name);
  if (!optionValue.isValid() || (optionValue.type() != QMetaType::Bool)) {
    qWarning() << "[OptionsManager] Cannot toggle non boolean or invalid option:" << settingKey(a_group, a_name);
    return false;
  }

  const bool toggledValue = !optionValue.toBool();

  const AffectsFlags flags = writeOption(a_group, a_name, toggledValue);
  applyFlags(flags);

  return toggledValue;
}

void OptionsManager::setValue(Group a_group, const QString& a_name, const QVariant& a_value)
{
  if (a_name.isEmpty()) {
    qWarning() << "[OptionsManager] Cannot set option without a name";
    return;
  }
  const AffectsFlags flags = writeOption(a_group, a_name, a_value);
  applyFlags(flags);
}

void OptionsManager::setValues(const OptionList& a_options)
{
  AffectsFlags flags{};
  for (const Option& option : a_options) {
    if (option.name.isEmpty()) {
      continue;
    }
    flags |= writeOption(option.group, option.name, option.value);
  }
  applyFlags(flags);
}

const QStringList& OptionsManager::recentsList() const
{
  return m_recentsList;
}

void OptionsManager::addRecentsEntry(const QString& a_entry) {
  m_recentsList.removeOne(a_entry);
  m_recentsList.prepend(a_entry);

  while (m_recentsList.count() > MAX_RECENTS_ENTRIES) {
    m_recentsList.removeLast();
  }

  m_settings->setValue(settingKey(Group::General, option::recentFiles), m_recentsList);
}

void OptionsManager::clearRecentsList()
{
  m_recentsList.clear();
  m_settings->setValue(settingKey(Group::General, option::recentFiles), m_recentsList);
}

void OptionsManager::applyFlags(AffectsFlags a_flags) const
{
  if (m_window == nullptr) {
    return;
  }
  if (a_flags.testFlag(Affects::Action) && m_window->actionsManager()) {
    m_window->actionsManager()->update();
  }
  if (a_flags.testFlag(Affects::View) && m_window->view()) {
    m_window->view()->scene()->readOptions();
  }
}

void OptionsManager::readOption(Group a_group, const QString& a_name, const QVariant a_default,
                                OptionEntry::AffectsFlags a_flags)
{
  const QString key = settingKey(a_group, a_name);
  const QVariant data = m_settings->value(key, a_default);
  auto type = data.type();
  m_optionsMap[key] = { data, a_flags };
}

OptionsManager::AffectsFlags OptionsManager::writeOption(Group a_group, const QString& a_name, const QVariant& a_value)
{
  const QString key = settingKey(a_group, a_name);

  OptionEntry& option = m_optionsMap[key];
  option.value = a_value;
  m_settings->setValue(key, a_value);

  return option.flags;
}

QString OptionsManager::settingKey(Group a_group, const QString& a_name)
{
  return QString{ "%1/%2" }.arg(GROUP_NAME.value(a_group, QStringLiteral("general"))).arg(a_name);
}

} // namespace qfsm
