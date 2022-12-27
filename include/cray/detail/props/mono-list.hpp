#pragma once

#include <concepts>
#include <vector>

#include "cray/detail/interval.hpp"
#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class MonoListAccessor {
   public:
	virtual Interval<std::size_t> const& getSize() const = 0;
};

template<typename V, std::derived_from<CodecProp<V>> P>
class MonoListProp
    : public CodecProp<std::vector<V>>
    , public MonoListAccessor {
   public:
	using StorageType      = std::vector<V>;
	using ChildPropType    = P;
	using ChildStorageType = V;

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
		return "List<" + this->next_prop->name() + ">";
	}

	bool ok() const override {
		if(!this->source->is(Type::List)) {
			return !this->isNeeded() || this->hasDefault();
		}

		if(auto const size = this->source->size(); !this->size.contains(size)) {
			return false;
		}

		return true;
	}

	void markRequired(Reference const& ref) override {
		throw InvalidAccessError();
	}

	std::shared_ptr<Prop> at(Reference const& ref) const override {
		return this->next_prop;
	}

	Interval<std::size_t> const& getSize() const override {
		return this->size;
	}

	std::shared_ptr<CodecProp<ChildStorageType>> next_prop;

	Interval<std::size_t> size;

   protected:
	void encodeTo_(Source& dst, StorageType const& value) const {
		for(std::size_t index = 0; index < value.size(); ++index) {
			this->next_prop->encodeTo(*this->source->next(index), value.at(index));
		}
	}

	bool decodeFrom_(Source const& src, StorageType& value) const {
		if(!this->source->is(Type::List)) {
			return false;
		}

		auto const size = this->source->size();
		if(!this->size.contains(size)) {
			return false;
		}

		value.resize(size);
		for(std::size_t index = 0; index < size; ++index) {
			auto const ok = this->next_prop->decodeFrom(*this->source->next(index), value.at(index));
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

}  // namespace detail
}  // namespace cray
