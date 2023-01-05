#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include "cray/detail/interval.hpp"
#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

template<typename V, std::derived_from<CodecProp<V>> P>
class MonoListProp
    : public IndexedPropHolder
    , public CodecProp<std::vector<V>> {
   public:
	using StorageType     = std::vector<V>;
	using NextPropType    = P;
	using NextStorageType = V;

	template<typename Ctx, bool = true>
	class DescriberBase: public detail::Describer<MonoListProp<V, P>> {
	   public:
		using detail::Describer<MonoListProp<V, P>>::Describer;
	};

	template<bool Dummy>
	class DescriberBase<GettableContext, Dummy>: public detail::Describer<MonoListProp<V, P>> {
	   public:
		using detail::Describer<MonoListProp<V, P>>::Describer;

		inline std::optional<StorageType> opt() const {
			return this->prop_->opt();
		}

		inline StorageType get() const {
			return this->prop_->get();
		}
	};

	template<typename Ctx>
	class Describer: public DescriberBase<Ctx> {
	   public:
		using ContextType = Ctx;

		using DescriberBase<Ctx>::DescriberBase;

		inline Describer const& defaultValue(StorageType value) const {
			this->prop_->default_value = std::move(value);
			return *this;
		}

		inline auto withDefault(StorageType value) const {
			this->defaultValue(std::move(value));
			if constexpr(std::is_same_v<Ctx, GettableContext>) {
				return this->prop_->get();
			} else {
				return *this;
			}
		}

		inline Describer const& size(Interval<std::size_t> interval) const {
			this->prop_->size = interval;
			return *this;
		}

		inline auto operator||(StorageType value) const {
			return this->withDefault(std::move(value));
		}
	};

	using CodecProp<std::vector<V>>::CodecProp;

	Type type() const override {
		return Type::List;
	}

	std::string name() const override {
		return "List of " + this->next_prop->name();
	}

	bool ok() const override {
		if(!this->source->is(Type::List)) {
			return !this->isNeeded() || this->hasDefault();
		}

		std::size_t const size = this->source->size();
		if(!this->interval().contains(size)) {
			return false;
		}

		for(std::size_t i = 0; i < size; ++i) {
			this->next_prop->source = this->source->next(i);

			bool const ok = this->next_prop->ok();
			if(!ok) {
				return false;
			}
		}

		return true;
	}

	bool needs(Reference const& ref) const {
		return false;
	}

	void markRequired(Reference const& ref) override {
	}

	void assign(Reference const& ref, std::shared_ptr<Prop> prop) override {
		auto next = std::dynamic_pointer_cast<CodecProp<NextStorageType>>(std::move(prop));
		if(next == nullptr) {
			throw InvalidAccessError();
		}

		this->next_prop = std::move(next);
	}

	std::shared_ptr<Prop> at(Reference const& ref) const override {
		return this->next_prop;
	}

	bool isMono() const override {
		return true;
	}

	Interval<std::size_t> interval() const override {
		return this->size;
	}

	std::shared_ptr<CodecProp<NextStorageType>> next_prop;

	Interval<std::size_t> size;

   protected:
	void encodeInto_(Source& dst, StorageType const& value) const {
		for(std::size_t index = 0; index < value.size(); ++index) {
			auto const next = dst.next(index);
			this->next_prop->encodeInto(*next, value.at(index));
		}
	}

	bool decodeFrom_(Source const& src, StorageType& value) const {
		if(!src.is(Type::List)) {
			return false;
		}

		auto const size = src.size();
		if(!this->interval().contains(size)) {
			return false;
		}

		value.resize(size);
		for(std::size_t index = 0; index < size; ++index) {
			auto const next = src.next(index);
			auto const ok   = this->next_prop->decodeFrom(*next, value.at(index));
			if(!ok) {
				return false;
			}
		}

		return true;
	}
};

template<std::derived_from<Prop> P>
using MonoListPropOf = MonoListProp<typename P::StorageType, P>;

template<typename T>
struct PropFor_<std::vector<T>> {
	using type = MonoListPropOf<PropFor<T>>;
};

template<typename V, typename P>
struct IsMonoPropHolder_<MonoListProp<V, P>>: std::true_type { };

}  // namespace detail
}  // namespace cray
