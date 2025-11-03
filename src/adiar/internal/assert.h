#ifndef ADIAR_INTERNAL_ASSERT_H
#define ADIAR_INTERNAL_ASSERT_H

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace adiar
{
  // LCOV_EXCL_START

  struct assert_error
  {
  private:
    std::string _what;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit assert_error(const std::string& what)
      : _what(what)
    {}

    explicit assert_error(const std::string& file, const int line)
    {
      std::stringstream s;
      s << file << "::" << line;
      _what = s.str();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit assert_error(const std::string& file, const int line, const std::string& what)
    {
      std::stringstream s;
      s << file << "::" << line << ": " << what;
      _what = s.str();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Explanatory string
    ////////////////////////////////////////////////////////////////////////////////////////////////
    char*
    what()
    {
      return _what.data();
    }
  };

#ifdef __GNUC__ // GCC 4.8+, Clang, Intel and other compilers compatible with GCC (-std=c++0x or
                // above)
  // Macros based on:
  // - Assert with file information and messages: https://stackoverflow.com/a/37264642
  // - Variadic macro arguments: https://stackoverflow.com/a/11763277
#define adiar_assert_overload(_1, _2, NAME, ...) NAME
#define adiar_assert(...) \
  adiar_assert_overload(__VA_ARGS__, adiar_assert2, adiar_assert1)(__VA_ARGS__)

#ifndef NDEBUG
#define adiar_assert1(Expr) __adiar_assert(#Expr, Expr, __FILE__, __LINE__)
#define adiar_assert2(Expr, Msg) __adiar_assert(#Expr, Expr, __FILE__, __LINE__, Msg)

  inline void
  __adiar_assert(const char* expr_str, bool expr, const char* file, int line)
  {
    if (!expr) {
      std::cerr << "Assert failed!\n"
                << "Expected:\t" << expr_str << "\n"
                << "Source:\t\t" << file << ", line " << line << std::endl;

      throw assert_error(file, line);
    }
  }

  inline void
  __adiar_assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
  {
    if (!expr) {
      std::cerr << "Assert failed:\t" << msg << "\n"
                << "Expected:\t" << expr_str << "\n"
                << "Source:\t\t" << file << ", line " << line << std::endl;

      throw assert_error(file, line, msg);
    }
  }
#else
#define adiar_assert1(Expr) ;
#define adiar_assert2(Expr, Msg) ;
#endif

#else // MSVC and ??? compilers
  inline void
  adiar_assert([[maybe_unused]] const bool expr, [[maybe_unused]] const std::string& what = "")
  {
#ifndef NDEBUG
    if (!expr) { throw assert_error(what); }
#endif
  }
#endif

  // LCOV_EXCL_STOP
}

#endif // ADIAR_INTERNAL_ASSERT_H
