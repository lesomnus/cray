#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "cray/detail/prop.hpp"
#include "cray/detail/props/poly-map.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {

template<typename M>
struct FieldContext {
	using MappedType = M;
};

template<typename M, typename V>
class FieldProp
    : public CodecProp<M>
    , public PropFor<V> {
   public:
	using MappedType  = M;
	using StorageType = V;

	using BasePropType    = PropFor<V>;
	using BaseStorageType = typename PropFor<V>::StorageType;

	FieldProp(Annotation annotation, std::string name, StorageType MappedType::*member)
	    : Prop(std::move(annotation), std::weak_ptr<Prop>(), std::move(name))
	    , member(member) { }

	std::string name() const override {
		return BasePropType::name();
	}

	bool hasDefault() const override {
		return BasePropType::hasDefault();
	}

	void encodeDefaultValueInto(Source& dst) const override {
		return BasePropType::encodeDefaultValueInto(dst);
	}

	void makeRequired() const override {
		if constexpr(!IsOptional<V>) {
			BasePropType::makeRequired();
		}
	};

	StorageType MappedType::*member;

   protected:
	void encodeInto_(Source& dst, MappedType const& value) const override {
		auto next = dst.next(this->ref);

		BaseStorageType v;
		if constexpr(IsOptional<V>) {
			if(value.*this->member) {
				v = *(value.*this->member);
			} else {
				return;
			}
		} else {
			v = value.*this->member;
		}

		CodecProp<BaseStorageType>::encodeInto(*next, v);
	}

	bool decodeFrom_(Source const& src, MappedType& value) const override {
		auto const next = src.next(this->ref);
		if(next == nullptr) {
			return false;
		}

		if constexpr(IsOptional<V>) {
			BaseStorageType v;
			if(CodecProp<BaseStorageType>::decodeFrom(*next, v)) {
				value.*this->member = v;
				return true;
			} else {
				return false;
			}
		} else {
			return CodecProp<BaseStorageType>::decodeFrom(*next, value.*this->member);
		}
	}
};

template<typename V>
class StructuredProp
    : public BasicPolyMapProp<CodecProp<V>>
    , public CodecProp<V> {
   public:
	using StorageType = V;

	template<typename Ctx, bool = true>
	class DescriberBase: public detail::Describer<StructuredProp<V>> {
	   public:
		using detail::Describer<StructuredProp<V>>::Describer;
	};

	template<bool Dummy>
	class DescriberBase<GettableContext, Dummy>: public detail::Describer<StructuredProp<V>> {
	   public:
		using detail::Describer<StructuredProp<V>>::Describer;

		inline std::optional<StorageType> opt() const {
			return this->prop_->opt();
		}

		inline StorageType get() const {
			return this->prop_->get();
		}

		inline std::optional<StorageType> operator&&(OptGetterQuery) const {
			return this->opt();
		}

		inline StorageType operator&&(GetterQuery) const {
			return this->get();
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

		template<WithContext<FieldContext<V>> D>
		inline Describer const& operator|(D describer) const {
			auto codec  = getProp(std::move(describer));
			codec->prev = this->prop_;
			codec->makeRequired();

			auto const ref = codec->ref;
			this->prop_->assign(ref, std::move(codec));

			return *this;
		};

		inline auto operator||(StorageType value) const {
			return this->withDefault(std::move(value));
		}
	};

	using CodecProp<V>::CodecProp;

	std::string name() const override {
		return this->annotation.title;
	}

   protected:
	void encodeInto_(Source& dst, StorageType const& value) const override {
		for(auto const& [key, next_prop]: this->next_props) {
			next_prop->encodeInto(dst, value);
		}
	}

	bool decodeFrom_(Source const& src, StorageType& value) const override {
		for(auto const& [key, next_prop]: this->next_props) {
			// No need to src.next(key) since codec knows their ref and
			// will navigate the Source with that ref.
			if(next_prop->decodeFrom(src, value)) {
				continue;
			}

			if(!this->required_keys.contains(key)) {
				continue;
			}

			return false;
		}

		return true;
	}
};

template<typename T>
struct PropFor_ {
	using type = StructuredProp<T>;
};

}  // namespace detail

template<typename M, typename V, typename Next = detail::PropFor<V>::NextPropType>
inline auto field(Annotation annotation, std::string name, V M::*member, detail::DescriberOf<Next, detail::EmptyContext> next) {
	using P = detail::FieldProp<M, V>;
	using D = detail::DescriberOf<P, detail::FieldContext<M>>;

	auto prop = std::make_shared<P>(std::move(annotation), std::move(name), member);
	prop->assign(Reference(), detail::getProp(std::move(next)));

	return D(std::move(prop));
}

template<typename M, typename V, typename Next = detail::PropFor<V>::NextPropType>
inline auto field(std::string name, V M::*member, detail::DescriberOf<Next, detail::EmptyContext> next) {
	return field(Annotation{}, std::move(name), member, std::move(next));
}

template<typename M, typename V>
inline auto field(Annotation annotation, std::string name, V M::*member) {
	using P = detail::FieldProp<M, V>;
	using D = detail::DescriberOf<P, detail::FieldContext<M>>;

	auto prop = std::make_shared<P>(std::move(annotation), std::move(name), member);
	detail::initPropRecursive(std::static_pointer_cast<detail::PropFor<V>>(prop));

	return D(std::move(prop));
}

template<typename M, typename V>
inline auto field(std::string name, V M::*member) {
	return field(Annotation{}, std::move(name), member);
}

}  // namespace cray
