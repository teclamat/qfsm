#ifndef HASH_HPP
#define HASH_HPP

#include <QStringView>

#include <cstddef>

namespace qfsm {

constexpr std::size_t HASH_VALUE{ 0xcbf29ce484222325 };
constexpr std::size_t HASH_PRIME{ 0x100000001b3 };

using Hash = std::size_t;

constexpr Hash hash(const char* const a_string, const std::size_t a_value = HASH_VALUE) noexcept
{
  return (*a_string == '\0') ? a_value
                             : hash(a_string + 1, (a_value ^ static_cast<std::size_t>(*a_string)) * HASH_PRIME);
}

constexpr Hash hash(const char16_t* const a_string, const std::size_t a_value = HASH_VALUE) noexcept
{
  return (*a_string == u'\0') ? a_value
                              : hash(a_string + 1, (a_value ^ static_cast<std::size_t>(*a_string)) * HASH_PRIME);
}

constexpr Hash hash(const char16_t* const a_begin, const char16_t* const a_end,
                    const std::size_t a_value = HASH_VALUE) noexcept
{
  return (a_begin == a_end) ? a_value
                            : hash(a_begin + 1, a_end, (a_value ^ static_cast<std::size_t>(*a_begin)) * HASH_PRIME);
}

} // namespace qfsm

#endif // HASH_HPP
