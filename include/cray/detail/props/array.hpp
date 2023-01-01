#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <utility>

#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

template<typename V, std::size_t N, std::derived_from<CodecProp<V>> P>
class ArrayProp
    : public CodecProp<std::array<V, N>>
    , public IndexedPropHolder {
   public:
	using StorageType     = std::array<V, N>;
	using NextPropType    = P;
	using NextStorageType = V;

	template<typename Ctx, bool = true>
	class DescriberBase: public detail::Describer<ArrayProp<V, N, P>> {
	   public:
		using detail::Describer<ArrayProp<V, N, P>>::Describer;
	};

	template<bool Dummy>
	class DescriberBase<GettableContext, Dummy>: public detail::Describer<ArrayProp<V, N, P>> {
	   public:
		using detail::Describer<ArrayProp<V, N, P>>::Describer;

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

		inline auto operator||(StorageType value) const {
			return this->withDefault(std::move(value));
		}
	};

	using CodecProp<std::array<V, N>>::CodecProp;

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

		if(this->source->size() != N) {
			return false;
		}

		return true;
	}

	bool needs(Reference const& ref) const {
		if(!ref.isIndex()) {
			return false;
		}

		return ref.index() < N;
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
		return Interval<std::size_t>::Singleton(N);
	}

	std::shared_ptr<CodecProp<NextStorageType>> next_prop;

   protected:
	void encodeInto_(Source& dst, StorageType const& value) const {
		for(std::size_t index = 0; index < value.size(); ++index) {
			auto const next_src = dst.next(index);
			this->next_prop->encodeInto(*next_src, value.at(index));
		}
	}

	bool decodeFrom_(Source const& src, StorageType& value) const {
		if(!src.is(Type::List)) {
			return false;
		}

		if(src.size() != N) {
			return false;
		}

		for(std::size_t i = 0; i < N; ++i) {
			auto const next_src = src.next(i);
			auto const ok       = this->next_prop->decodeFrom(*next_src, value.at(i));
			if(!ok) {
				return false;
			}
		}

		return true;
	}
};

template<std::derived_from<Prop> P, std::size_t N>
using ArrayPropOf = ArrayProp<typename P::StorageType, N, P>;

template<typename V, std::size_t N>
struct PropFor_<std::array<V, N>> {
	using type = ArrayPropOf<PropFor<V>, N>;
};

template<typename V, std::size_t N, typename P>
struct IsMonoPropHolder_<ArrayProp<V, N, P>>: std::true_type { };

}  // namespace detail
}  // namespace cray
