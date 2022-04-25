#ifndef SRC_LITERALS_HPP
#define SRC_LITERALS_HPP

#include "hash.hpp"

#include <QString>

#include <cstddef>

// Adds QStringLiteral literal for earlier than 6.2 Qt versions.
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
inline QString operator"" _qs(const char16_t* a_string, std::size_t a_size) noexcept
{
  return QString{ QStringPrivate{ nullptr, const_cast<char16_t*>(str), static_cast<qsizetype>(size) } };
}
#endif

constexpr qfsm::Hash operator""_hash(const char* a_string, std::size_t) noexcept
{
  return qfsm::hash(a_string);
}

constexpr qfsm::Hash operator""_hash(const char16_t* a_string, std::size_t) noexcept
{
  return qfsm::hash(a_string);
}

#endif // SRC_LITERALS_HPP
