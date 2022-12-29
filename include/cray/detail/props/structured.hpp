#pragma once

#include <cassert>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "cray/detail/ordered_map.hpp"
#include "cray/detail/ordered_set.hpp"
#include "cray/detail/prop.hpp"
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

class StructuredPropAccessor {
   public:
	virtual std::size_t getSize() const = 0;

	virtual void getNextProps(std::function<bool(std::string const&, std::shared_ptr<Prop> const&)> const& functor) const = 0;
};

template<typename V>
class StructuredProp
    : public CodecProp<V>
    , public StructuredPropAccessor {
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

	Type type() const {
		return Type::Map;
	}

	std::string name() const {
		return this->annotation.title;
	}

	bool ok() const override {
		if(!this->source->is(Type::Map)) {
			return !this->isNeeded() || this->hasDefault();
		}

		for(auto const& [key, codec]: this->codecs) {
			codec->source = this->source->next(key);
			if(codec->ok()) {
				continue;
			}

			return false;
		}

		return true;
	}

	void markRequired(Reference const& ref) override {
		this->required_keys.insert(ref.key());
	};

	bool needs(Reference const& ref) const {
		return this->required_keys.contains(ref.key());
	}

	void assign(Reference const& ref, std::shared_ptr<Prop> prop) override {
		auto codec = std::dynamic_pointer_cast<CodecProp<V>>(std::move(prop));
		assert(codec != nullptr);

		auto const& key = ref.key();
		this->codecs.insert_or_assign(key, std::move(codec));
	}

	std::shared_ptr<Prop> at(Reference const& ref) const override {
		auto const it = this->codecs.find(ref.key());
		if(it == this->codecs.cend()) {
			return nullptr;
		} else {
			return it->second;
		}
	}

	std::size_t getSize() const override {
		return this->codecs.size();
	}

	void getNextProps(std::function<bool(std::string const&, std::shared_ptr<Prop> const&)> const& functor) const override {
		for(auto const& [key, codec]: codecs) {
			functor(key, codec);
		}
	}

	OrderedMap<std::string, std::shared_ptr<CodecProp<StorageType>>> codecs;
	OrderedSet<std::string>                                          required_keys;

   protected:
	void encodeInto_(Source& dst, StorageType const& value) const override {
		for(auto const& [key, codec]: this->codecs) {
			codec->encodeInto(dst, value);
		}
	}

	bool decodeFrom_(Source const& src, StorageType& value) const override {
		for(auto const& [key, codec]: this->codecs) {
			// No need to src.next(key) since codec knows their ref and
			// will navigate the Source with that ref.
			if(codec->decodeFrom(src, value)) {
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
