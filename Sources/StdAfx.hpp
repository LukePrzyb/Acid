#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#if defined(ACID_STATIC_DEFINE)
#  define ACID_EXPORT
#  define ACID_NO_EXPORT
#else
#  if defined(ACID_BUILD_MSVC)
#    if defined(ACID_EXPORTS)
#      define ACID_EXPORT __declspec(dllexport)
#    else
#      define ACID_EXPORT __declspec(dllimport)
#    endif
#    define ACID_NO_EXPORT 
#  else
#    define ACID_EXPORT __attribute__((visibility("default")))
#    define ACID_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#if defined(ACID_BUILD_MSVC)
#  define ACID_DEPRECATED __declspec(deprecated)
#else
#  define ACID_DEPRECATED __attribute__ ((__deprecated__))
#endif
