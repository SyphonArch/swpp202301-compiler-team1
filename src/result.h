#ifndef SC_RESULT_H
#define SC_RESULT_H

/**
 * @file result.h
 * @author SWPP TAs (swpp@sf.snu.ac.kr)
 * @brief Header-only module for monadic results
 * @version 2023.1.1
 * @date 2023-04-16
 * @copyright Copyright (c) 2022-2023 SWPP TAs
 */

#include "static_error.h"

#include <functional>
#include <optional>
#include <type_traits>
#include <variant>

namespace result {
/**
 * @brief Exception thrown by trying to unwrap the Err Result
 */
class BadUnwrapError : public Error<BadUnwrapError> {
public:
  /**
   * @brief Read the exception
   * @return Exception message in C-String format
   */
  const char *what() const noexcept {
    return "Result error: tried to unwrap Err";
  }
};

/**
 * @brief Exception thrown by trying to inspect the Ok Result
 */
class BadInspectError : public Error<BadInspectError> {
public:
  /**
   * @brief Read the exception
   * @return Exception message in C-String format
   */
  const char *what() const noexcept {
    return "Result error: tried to inspect Ok";
  }
};

/**
 * @brief Exception thrown by trying to access the 'moved away' Result
 */
class InvalidResultError : public Error<InvalidResultError> {
public:
  /**
   * @brief Read the exception
   * @return Exception message in C-String format
   */
  const char *what() const noexcept {
    return "Result error: tried to access already moved Result";
  }
};

/**
 * @brief Monadic result type for error handling without runtime overhead
 *
 * This represents the output of 'potentially failing' operations.
 *
 * Result can have one of the two states: Ok and Err.
 * * Ok contains the output from successful operation. The output can be later
 * extracted using the `unwrap()`.
 * * Err contains the error from unsuccessful operation. The error can be later
 * extracted using the `inspect()`.
 *
 * Monadic methods can be used to modify the output or propagte the error.
 * You can use these methods even if you don't understand the mathematical
 * definition of monad. Actually, in our context, it is just a fancy way of
 * saying "conditional apply".
 *
 * However, due to limitations in the c++ grammar, it is hard to catch the
 * use of 'moved' Results at compile time, which must be checked at runtime
 * and adds some overhead. Also, the template type deduction does not work
 * very well on Result types, so type information must always be provided
 * when calling the monads.
 * ```
 * auto result = operationThatYieldsSomeResult();
 * auto mapped_result = result.map<U>([](auto &&data){
     return data.convertToU(data);
   });
   // at this point any attempt to access `result` will raise exception
 * const U expected = mapped_result.unwrap();
 * ```
 *
 * Also, you may use the 'cp' variants of these monads if your types
 * does not properly support moving or you have to reuse the Results.
 *
 * @tparam T type of return value upon success
 * @tparam E type of error upon failure
 */
template <typename T, typename E> class Result {
  using DataTy = std::variant<T, E>;

private:
  enum class Kind { OK, ERR, MOVED };
  Kind kind;
  DataTy data;

  /**
   * @brief Construct a new Result object
   */
  constexpr Result(const Kind kind, DataTy &&data) noexcept
      : kind(kind), data(std::move(data)) {}

  /**
   * @brief Check if the current Result is not 'moved from'
   * @return true if the result has not been moved
   */
  constexpr bool isValid() const noexcept { return kind != Kind::MOVED; }

  /**
   * @brief throw if the current Result is invalid
   * @throws InvalidResultError if it is moved Result
   */
  constexpr void throwIfInvalid() const {
    if (!isValid()) {
      throw InvalidResultError();
    }
  }

  /**
   * @brief Check if all monad member types are movable
   *
   * @tparam U return type of monad argument function
   * @return true if the types T, E, U are all movable
   */
  template <typename U> constexpr static bool monadTypesAreMovable() noexcept {
    constexpr auto trait_T =
        std::is_move_constructible_v<T> && std::is_move_assignable_v<T>;
    constexpr auto trait_E =
        std::is_move_constructible_v<E> && std::is_move_assignable_v<E>;
    constexpr auto trait_U =
        std::is_move_constructible_v<U> && std::is_move_assignable_v<U>;
    return trait_T && trait_E && trait_U;
  }

  /**
   * @brief Check if all monad member types are copyable
   *
   * @tparam U return type of monad argument function
   * @return true if the types T, E, U are all copyable
   */
  template <typename U> constexpr static bool monadTypesAreCopyable() noexcept {
    constexpr auto trait_T =
        std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>;
    constexpr auto trait_E =
        std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>;
    constexpr auto trait_U =
        std::is_copy_constructible_v<U> && std::is_copy_assignable_v<U>;
    return trait_T && trait_E && trait_U;
  }

  /**
   * @brief Check if the monad is valid and all monad member types are movable
   *
   * @tparam U return type of monad argument function
   * @throws InvalidResultError if the Result is in invalid state
   */
  template <typename U> constexpr void checkMovingMonad() const {
    static_assert(Result::monadTypesAreMovable<U>());
    throwIfInvalid();
  }

  /**
   * @brief Check if the monad is valid and all monad member types are copyable
   *
   * @tparam U return type of monad argument function
   * @throws InvalidResultError if the Result is in invalid state
   */
  template <typename U> constexpr void checkCopyingMonad() const {
    static_assert(Result::monadTypesAreCopyable<U>());
    throwIfInvalid();
  }

public:
  /**
   * @brief Move-construct a Result object
   * @param other Result to move from
   */
  constexpr Result(Result &&other) noexcept = default;

  /**
   * @brief Move-assign a Result object
   * @param other Result to move from
   */
  constexpr Result &operator=(Result &&other) noexcept = default;

  /**
   * @brief Check if the Result can be unwrapped
   * @return true if the Result is Ok
   * @return false otherwise
   */
  constexpr bool isOk() const noexcept { return (kind == Kind::OK); }

  /**
   * @brief Check if the Result can be inspected
   * @return true if the Result is Err
   * @return false otherwise
   */
  constexpr bool isErr() const noexcept { return (kind == Kind::ERR); }

  /**
   * @brief Create a new Ok Result
   * @param data output to move into the Result
   * @return constexpr Result of Ok state
   */
  constexpr static Result Ok(T &&data) noexcept {
    return Result(Kind::OK, std::move(data));
  }

  /**
   * @brief Create a new Ok Result by copying
   * @param data output to copy into the Result
   * @return constexpr Result of Ok state
   */
  constexpr static Result Ok(const T &data) {
    return Result(Kind::OK, data);
  }

  /**
   * @brief Create a new Err Result
   * @param data output to move into the Result
   * @return constexpr Result of Ok state
   */
  constexpr static Result Err(E &&data) noexcept {
    return Result(Kind::ERR, std::move(data));
  }

  /**
   * @brief Create a new Err Result by copying
   * @param data output to copy into the Result
   * @return constexpr Result of Ok state
   */
  constexpr static Result Err(const E &data) {
    return Result(Kind::ERR, data);
  }

  /**
   * @brief Apply the given function to the Ok value, while leaving the Err
   * Result untouched
   *
   * This monad computes a `Result<U, E>` from `Result<T, E>` using `U fn(&&T)`
   * by moving the contained value or error out of the original Result,
   * putting the source Result in an invalid state.
   * * If the Result was Ok, apply `fn` to the contained value, and create
   * a new Ok Result out of it
   * * If the Result was Err, type-cast a new Err Result without touching the
   * contained error
   *
   * @tparam U return type of `fn`
   * @param fn Function used to map the value
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr Result<U, E> See the description
   */
  template <typename U>
  constexpr Result<U, E> map(std::function<U(T &&)> &&fn) {
    checkMovingMonad<U>();

    std::optional<Result<U, E>> res;
    if (isOk()) {
      auto data = std::get<T>(std::move(this->data));
      res = Result<U, E>::Ok(fn(std::move(data)));
    } else {
      auto data = std::get<E>(std::move(this->data));
      res = Result<U, E>::Err(std::move(data));
    }

    kind = Kind::MOVED;
    return *std::move(res);
  }

  /**
   * @brief Apply the given function to the Ok value, while leaving the Err
   * Result untouched
   *
   * This monad computes a `Result<U, E>` from `Result<T, E>` using `U fn(&&T)`
   * by copying the contained value or error out of the original Result,
   * leaving the source Result 'as is'.
   * * If the Result was Ok, apply `fn` to the contained value, and create
   * a new Ok Result out of it
   * * If the Result was Err, type-cast a new Err Result without touching the
   * contained error
   *
   * @tparam U return type of `fn`
   * @param fn Function used to map the value
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr Result<U, E> See the description
   */
  template <typename U>
  constexpr Result<U, E> cpMap(std::function<U(const T &)> &&fn) const {
    checkCopyingMonad<U>();

    if (isOk()) {
      auto data = std::get<T>(this->data);
      return Result<U, E>::Ok(fn(data));
    } else {
      auto data = std::get<E>(this->data);
      return Result<U, E>::Err(data);
    }
  }

  /**
   * @brief Apply the given function to the Err value, while leaving the Ok
   * Result untouched
   *
   * This monad computes a `Result<T, F>` from `Result<T, E>` using `F fn(&&E)`
   * by moving the contained value or error out of the original Result,
   * putting the source Result in an invalid state.
   * * If the Result was Err, apply `fn` to the contained error, and create
   * a new Err Result out of it
   * * If the Result was Ok, type-cast a new Ok Result without touching the
   * contained value
   *
   * @tparam F return type of `fn`
   * @param fn Function used to map the error
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr Result<T, F> See the description
   */
  template <typename F>
  constexpr Result<T, F> mapErr(std::function<F(E &&)> &&fn) {
    checkMovingMonad<F>();

    std::optional<Result<T, F>> res;
    if (isErr()) {
      auto data = std::get<E>(std::move(this->data));
      res = Result<T, F>::Err(fn(std::move(data)));
    } else {
      auto data = std::get<T>(std::move(this->data));
      res = Result<T, F>::Ok(std::move(data));
    }

    kind = Kind::MOVED;
    return *std::move(res);
  }

  /**
   * @brief Apply the given function to the Err value, while leaving the Ok
   * Result untouched
   *
   * This monad computes a `Result<T, F>` from `Result<T, E>` using `F fn(&&E)`
   * by copying the contained value or error out of the original Result,
   * leaving the source Result 'as is'.
   * * If the Result was Err, apply `fn` to the contained error, and create
   * a new Err Result out of it
   * * If the Result was Ok, type-cast a new Ok Result without touching the
   * contained value
   *
   * @tparam F return type of `fn`
   * @param fn Function used to map the error
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr Result<T, F> See the description
   */
  template <typename F>
  constexpr Result<T, F> cpMapErr(std::function<F(const E &)> &&fn) const {
    checkCopyingMonad<F>();

    if (isErr()) {
      auto data = std::get<E>(this->data);
      return Result<T, F>::Err(fn(data));
    } else {
      auto data = std::get<T>(this->data);
      return Result<T, F>::Ok(data);
    }
  }

  /**
   * @brief Compute new Result using the given function if the source is Ok,
   * or type-cast the source if it is Err
   *
   * This monad computes a `Result<U, E>` from `Result<T, E>`
   * using `Result<U, E> fn(&&T)`
   * by moving the contained value or error out of the original Result,
   * putting the source Result in an invalid state.
   * * If the Result was Ok, return the Result obtained by applying `fn`
   * to the contained value
   * * If the Result was Err, type-cast a new Err Result without touching the
   * contained error
   *
   * @tparam U Ok type from return type of `fn`
   * @param fn Function used to map the value
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr Result<U, E> See the description
   */
  template <typename U>
  constexpr Result<U, E> andThen(std::function<Result<U, E>(T &&)> &&fn) {
    checkMovingMonad<U>();

    std::optional<Result<U, E>> res;
    if (isOk()) {
      auto data = std::get<T>(std::move(this->data));
      res = fn(std::move(data));
    } else {
      auto data = std::get<E>(std::move(this->data));
      res = Result<U, E>::Err(std::move(data));
    }

    kind = Kind::MOVED;
    return *std::move(res);
  }

  /**
   * @brief Compute new Result using the given function if the source is Ok,
   * or type-cast the source if it is Err
   *
   * This monad computes a `Result<U, E>` from `Result<T, E>`
   * using `Result<U, E> fn(&&T)`
   * by copying the contained value or error out of the original Result,
   * leaving the source Result 'as is'.
   * * If the Result was Ok, return the Result obtained by applying `fn`
   * to the contained value
   * * If the Result was Err, type-cast a new Err Result without touching the
   * contained error
   *
   * @tparam U Ok type from return type of `fn`
   * @param fn Function used to map the value
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr Result<U, E> See the description
   */
  template <typename U>
  constexpr Result<U, E>
  cpAndThen(std::function<Result<U, E>(const T &)> &&fn) const {
    checkCopyingMonad<U>();

    if (isOk()) {
      auto data = std::get<T>(this->data);
      return fn(data);
    } else {
      auto data = std::get<E>(this->data);
      return Result<U, E>::Err(data);
    }
  }

  /**
   * @brief Compute new Result using the given function if the source is Err,
   * or type-cast the source if it is Ok
   *
   * This monad computes a `Result<T, F>` from `Result<T, E>`
   * using `Result<T, F> fn(&&E)`
   * by moving the contained value or error out of the original Result,
   * putting the source Result in an invalid state.
   * * If the Result was Err, return the Result obtained by applying `fn`
   * to the contained error
   * * If the Result was Ok, type-cast a new Ok Result without touching the
   * contained value
   *
   * @tparam F Err type from return type of `fn`
   * @param fn Function used to map the error
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr Result<T, F> See the description
   */
  template <typename F>
  constexpr Result<T, F> orElse(std::function<Result<T, F>(E &&)> &&fn) {
    checkMovingMonad<F>();

    std::optional<Result<T, F>> res;
    if (isErr()) {
      auto data = std::get<E>(std::move(this->data));
      res = fn(std::move(data));
    } else {
      auto data = std::get<T>(std::move(this->data));
      res = Result<T, F>::Ok(std::move(data));
    }

    kind = Kind::MOVED;
    return *std::move(res);
  }

  /**
   * @brief Compute new Result using the given function if the source is Err,
   * or type-cast the source if it is Ok
   *
   * This monad computes a `Result<T, F>` from `Result<T, E>`
   * using `Result<T, F> fn(&&E)`
   * by copying the contained value or error out of the original Result,
   * leaving the source Result 'as is'.
   * * If the Result was Err, return the Result obtained by applying `fn`
   * to the contained error
   * * If the Result was Ok, type-cast a new Ok Result without touching the
   * contained value
   *
   * @tparam F Err type from return type of `fn`
   * @param fn Function used to map the error
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr Result<T, F> See the description
   */
  template <typename F>
  constexpr Result<T, F>
  cpOrElse(std::function<Result<T, F>(const E &)> &&fn) const {
    checkCopyingMonad<F>();

    if (isErr()) {
      auto data = std::get<E>(this->data);
      return fn(data);
    } else {
      auto data = std::get<T>(this->data);
      return Result<T, F>::Ok(data);
    }
  }

  /**
   * @brief Compute new value from Result of any state using two given functions
   *
   * This monad computes a `U` from `Result<T, E>`
   * using two functions `U fn(&&T)` and `U fb(&&E)`
   * by moving the contained value or error out of the original Result,
   * putting the source Result in an invalid state.
   * * If the Result was Ok, return the output obtained by applying `fn`
   * to the contained value
   * * If the Result was Err, return the output obtained by applying `fb`
   * to the contained error
   *
   * @tparam U Return type of `fb` and `fn`
   * @param fb Function used to map the error
   * @param fn Function used to map the value
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr U See the description
   */
  template <typename U>
  constexpr U mapOrElse(std::function<U(E &&)> &&fb,
                        std::function<U(T &&)> &&fn) {
    checkMovingMonad<U>();

    std::optional<U> res;
    if (isOk()) {
      auto data = std::get<T>(std::move(this->data));
      res = fn(std::move(data));
    } else {
      auto data = std::get<E>(std::move(this->data));
      res = fb(std::move(data));
    }

    kind = Kind::MOVED;
    return *std::move(res);
  }

  /**
   * @brief Compute new value from Result of any state using two given functions
   *
   * This monad computes a `U` from `Result<T, E>`
   * using two functions `U fn(&&T)` and `U fb(&&E)`
   * by copying the contained value or error out of the original Result,
   * leaving the source Result 'as is'.
   * * If the Result was Ok, return the output obtained by applying `fn`
   * to the contained value
   * * If the Result was Err, return the output obtained by applying `fb`
   * to the contained error
   *
   * @tparam U Return type of `fb` and `fn`
   * @param fb Function used to map the error
   * @param fn Function used to map the value
   * @throws InvalidResultError if the Result is in invalid state
   * @return constexpr U See the description
   */
  template <typename U>
  constexpr U cpMapOrElse(std::function<U(const E &)> &&fb,
                          std::function<U(const T &)> &&fn) const {
    checkCopyingMonad<U>();

    if (isOk()) {
      auto data = std::get<T>(this->data);
      return fn(data);
    } else {
      auto data = std::get<E>(this->data);
      return fb(data);
    }
  }

  /**
   * @brief Extract the contained value from Ok Result
   *
   * This monad extracts the contained value from `Result<T, E>`
   * that is expected to be Ok and puts the source Result in an invalid state.
   * You should make sure that the Result actually is Ok prior to using this
   * monad. One way to achieve it is using `isOk()`. Exception will be thrown
   * upon trying to unwrap the Err or invalid Result.
   *
   * @return constexpr T Contained output
   * @throws InvalidResultError if the Result is in invalid state
   * @throws BadUnwrapError if `src` is Err Result
   */
  constexpr T unwrap() {
    checkMovingMonad<T>();

    if (!isOk()) {
      throw BadUnwrapError();
    }
    kind = Kind::MOVED;
    return std::get<T>(std::move(data));
  }

  /**
   * @brief Copy the contained value from Ok Result
   *
   * This monad copies the contained value from `Result<T, E>`
   * that is expected to be Ok. You should make sure that the Result actually
   * is Ok prior to using this monad. One way to achieve it is using `isOk()`.
   * Exception will be thrown upon trying to unwrap the Err or invalid Result.
   *
   * @return constexpr T Contained output
   * @throws InvalidResultError if the Result is in invalid state
   * @throws BadUnwrapError if `src` is Err Result
   */
  constexpr T cpUnwrap() const {
    checkCopyingMonad<T>();

    if (!isOk()) {
      throw BadUnwrapError();
    }
    return std::get<T>(data);
  }

  /**
   * @brief Extract the error from Err Result
   *
   * This monad extracts the error from `Result<T, E>`
   * that is expected to be Err and puts the source Result in an invalid state.
   * You should make sure that the Result actually is Err prior to using this
   * monad. One way to achieve it is using `isErr()`. Exception will be thrown
   * upon trying to inspect the Ok Result.
   *
   * @return constexpr E Contained error
   * @throws InvalidResultError if the Result is in invalid state
   * @throws BadInspectError if `src` is Ok Result
   */
  constexpr E inspect() {
    checkMovingMonad<T>();

    if (!isErr()) {
      throw BadInspectError();
    }
    kind = Kind::MOVED;
    return std::get<E>(std::move(data));
  }

  /**
   * @brief Copy the error from Err Result
   *
   * This monad copies the error from `Result<T, E>`
   * that is expected to be Err. You should make sure that the Result actually
   * is Err prior to using this monad. One way to achieve it is using `isErr()`.
   * Exception will be thrown upon trying to inspect the Ok Result.
   *
   * @return constexpr E Contained error
   * @throws InvalidResultError if the Result is in invalid state
   * @throws BadInspectError if `src` is Ok Result
   */
  constexpr E cpInspect() const {
    checkCopyingMonad<T>();

    if (!isErr()) {
      throw BadInspectError();
    }
    return std::get<E>(data);
  }

  /**
   * @brief Extract the output from Ok Result, or compute one from the error in
   * Err Result using the given function
   *
   * This monad computes a `T` from `Result<T, E>`
   * using `T fn(&&E)` and puts the source Result in an invalid state.
   * * If the Result was Ok, simply extract the contained value
   * * If the Result was Err, return the output obtained by applying `fn`
   * to the contained error
   *
   * @param fn Function used to map the error
   * @return constexpr T See the description
   */
  constexpr T unwrapOrElse(std::function<T(E &&)> &&fn) {
    checkMovingMonad<T>();

    std::optional<T> res;
    if (isOk()) {
      res = std::get<T>(std::move(this->data));
    } else {
      auto data = std::get<E>(std::move(this->data));
      res = fn(std::move(data));
    }

    kind = Kind::MOVED;
    return *std::move(res);
  }

  /**
   * @brief Copy the output from Ok Result, or compute one from the error in
   * Err Result using the given function
   *
   * This monad computes a `T` from `Result<T, E>`
   * using `T fn(&&E)`.
   * * If the Result was Ok, simply extract the contained value
   * * If the Result was Err, return the output obtained by applying `fn`
   * to the contained error
   *
   * @param fn Function used to map the error
   * @return constexpr T See the description
   */
  constexpr T cpUnwrapOrElse(std::function<T(const E &)> &&fn) const {
    checkCopyingMonad<T>();

    if (isOk()) {
      return std::get<T>(this->data);
    } else {
      auto data = std::get<E>(this->data);
      return fn(data);
    }
  }
};
} // namespace result

template <typename T, typename E> using Result = result::Result<T, E>;
#endif // SC_RESULT_H
