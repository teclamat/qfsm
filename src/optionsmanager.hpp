#ifndef SRC_OPTIONSMANAGER_HPP
#define SRC_OPTIONSMANAGER_HPP

#include <QFlags>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>

class MainWindow;
class QSettings;
class QMainWindow;

namespace qfsm {

namespace option {

enum class Group { General, Print, View };

constexpr auto recentFiles = "recentFiles";
constexpr auto stateEncoding = "showStateEncoding";
constexpr auto mooreOutputs = "showMooreOutputs";
constexpr auto mealyInputs = "showMealyInputs";
constexpr auto mealyOutputs = "showMealyOutputs";
constexpr auto shadows = "showShadows";
constexpr auto shadowColor = "shadowColor";
constexpr auto grid = "showGrid";
constexpr auto gridSize = "gridSize";
constexpr auto gridColor = "gridColor";
constexpr auto ioView = "showIoView";

} // namespace option

struct Option {
  option::Group group;
  QString name;
  QVariant value;
};

using OptionList = QList<Option>;

struct OptionEntry {
  enum class Affects { Nothing = 0x00, View = 0x01, Action = 0x02 };
  using AffectsFlags = QFlags<Affects>;
  QVariant value;
  AffectsFlags flags;
};

class OptionsManager : public QObject {
  Q_OBJECT
 public:
  using OptionsMap = QHash<QString, OptionEntry>;
  using Group = option::Group;
  using Affects = OptionEntry::Affects;
  using AffectsFlags = OptionEntry::AffectsFlags;

  OptionsManager(QMainWindow* a_window = nullptr);
  const QVariant variant(Group a_group, const QString& a_name) const;

  template<typename T>
  T value(Group a_group, const QString& a_name) const
  {
    return qvariant_cast<T>(variant(a_group, a_name));
  }

  bool toggleValue(Group a_group, const QString& a_name);
  void setValue(Group a_group, const QString& a_name, const QVariant& a_value);
  void setValues(const OptionList& a_options);

  const QStringList& recentsList() const;
  void addRecentsEntry(const QString& a_entry);
  void removeRecentsEntry(const QString& a_entry);
  void clearRecentsList();

 private:
  void applyFlags(AffectsFlags a_flags) const;
  void readOption(Group a_group, const QString& a_name, const QVariant a_default, AffectsFlags a_flags = {});
  AffectsFlags writeOption(Group a_group, const QString& a_name, const QVariant& a_value);

  static QString settingKey(Group a_group, const QString& a_name);

 private:
  MainWindow* m_window;
  QSettings* m_settings;

  OptionsMap m_optionsMap{};
  QStringList m_recentsList{};
};

} // namespace qfsm

#endif // SRC_OPTIONSMANAGER_HPP
