#ifndef SC_STATIC_ERROR_H
#define SC_STATIC_ERROR_H

/**
 * @file static_error.h
 * @author SWPP TAs (swpp@sf.snu.ac.kr)
 * @brief Header-only module for statically typed exceptions
 * @version 2023.1.0
 * @date 2023-04-16
 * @copyright Copyright (c) 2022-2023 SWPP TAs
 */

#include <exception>
#include <memory>
#include <string>

namespace static_error {
/**
 * @brief CRTP interface for statically typed exceptions
 *
 * This provides some boilerplate for static exceptions.
 * Derived exceptions can be thrown and caught just like ordinary exceptions.
 * But because they are statically typed, they can be read without RTTI overhead
 * @tparam E Derived exception
 */
template <typename E> class Error : public std::exception {
private:
  /**
   * @brief Construct a new Error object
   */
  Error() noexcept {}
  friend E;

public:
  /**
   * @brief Read the exception
   * @return Exception message in C-String format
   */
  const char *what() const noexcept override {
    return static_cast<const E *>(this)->what();
  }

  /**
   * @brief Move-construct an Error object
   * @param other Error to move from
   */
  Error(Error &&other) noexcept = default;

  /**
   * @brief Move-assign an Error object
   * @param other Error to move from
   */
  Error &operator=(Error &&other) noexcept = default;
};
} // namespace static_error

template <typename E> using Error = static_error::Error<E>;
#endif // SC_STATIC_ERROR_H
