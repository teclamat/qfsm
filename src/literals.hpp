#ifndef SRC_LITERALS_HPP
#define SRC_LITERALS_HPP


// Adds QStringLiteral literal for earlier than 6.2 Qt versions.
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)

#include <QString>
#include <cstddef>

inline QString operator"" _qs(const char16_t* a_string, std::size_t a_size) noexcept
{
  return QString{ QStringPrivate{ nullptr, const_cast<char16_t*>(str), static_cast<qsizetype>(size) } };
}

#endif

#endif // SRC_LITERALS_HPP
