#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

namespace cray {

enum class Type {
	Unspecified,
	Nil,
	Bool,
	Int,
	Num,
	Str,
	Map,
	List,
};

namespace detail {

class InvalidAccessError: public std::logic_error {
   public:
	InvalidAccessError()
	    : std::logic_error("invalid access") { }
};

// clang-format off
template<Type T> struct IsScalarType_             { static constexpr bool value = false; };
template<>       struct IsScalarType_<Type::Nil>  { static constexpr bool value = true;  };
template<>       struct IsScalarType_<Type::Bool> { static constexpr bool value = true;  };
template<>       struct IsScalarType_<Type::Int>  { static constexpr bool value = true;  };
template<>       struct IsScalarType_<Type::Num>  { static constexpr bool value = true;  };
template<>       struct IsScalarType_<Type::Str>  { static constexpr bool value = true;  };

template<Type T> struct StorageOf_ { };
template<>       struct StorageOf_<Type::Nil>  { using type = std::nullptr_t; };
template<>       struct StorageOf_<Type::Bool> { using type = bool;           };
template<>       struct StorageOf_<Type::Int>  { using type = std::intmax_t;  };
template<>       struct StorageOf_<Type::Num>  { using type = double;         };
template<>       struct StorageOf_<Type::Str>  { using type = std::string;    };

template<typename T> struct StorageFor_                   { using type = T;                      };
template<typename T> struct StorageFor_<std::optional<T>> { using type = typename T::value_type; };

template<typename T> struct TypeFor_                 { static constexpr Type value = Type::Map;  };
template<>           struct TypeFor_<std::nullptr_t> { static constexpr Type value = Type::Nil;  };
template<>           struct TypeFor_<bool          > { static constexpr Type value = Type::Bool; };
template<>           struct TypeFor_<std::string   > { static constexpr Type value = Type::Str;  };

template<std::integral       T> struct TypeFor_<T> { static constexpr Type value = Type::Int; };
template<std::floating_point T> struct TypeFor_<T> { static constexpr Type value = Type::Num; };

template<typename T> struct IsOptional_                   { static constexpr bool value = false; };
template<typename T> struct IsOptional_<std::optional<T>> { static constexpr bool value = true;  };
// clang-format on

template<typename T>
constexpr bool IsOptional = IsOptional_<T>::value;

}  // namespace detail

template<Type T>
constexpr bool IsScalarType = detail::IsScalarType_<T>::value;

constexpr bool isScalarType(Type type) {
	switch(type) {
	case Type::Nil:
	case Type::Bool:
	case Type::Int:
	case Type::Num:
	case Type::Str:
		return true;

	default:
		return false;
	}
}

template<Type T>
using StorageOf = detail::StorageOf_<T>::type;

template<typename T>
using StorageFor = detail::StorageFor_<T>::type;

template<typename T>
constexpr Type TypeFor = detail::TypeFor_<T>::value;

struct Annotation {
	std::string title;
	std::string description;

	bool is_deprecated = false;
};

struct Key {
	std::string name;

	bool is_required = false;
};

class Reference {
   public:
	Reference() = default;

	Reference(std::size_t value)
	    : storage_(value) { }

	Reference(int value)
	    : storage_(static_cast<std::size_t>(value)) { }

	Reference(std::string value)
	    : storage_(std::move(value)) { }

	Reference(char const* value)
	    : storage_(std::string(value)) { }

	std::size_t index() const {
		return std::get<std::size_t>(storage_);
	}

	std::string const& key() const& {
		return std::get<std::string>(storage_);
	}

	std::string const& key() & {
		return std::get<std::string>(storage_);
	}

	std::string&& key() && {
		return std::move(std::get<std::string>(storage_));
	}

	bool isIndex() const {
		return std::holds_alternative<std::size_t>(storage_);
	}

   private:
	std::variant<std::size_t, std::string> storage_;
};

}  // namespace cray
