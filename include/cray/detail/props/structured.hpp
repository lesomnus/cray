#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {

template<typename M>
struct FieldContext {
	using MappedType = M;
};

template<typename M, typename V, typename P = PropFor<V>>
class FieldProp
    : public CodecProp<M>
    , public P {
   public:
	using MappedType   = M;
	using StorageType  = V;
	using ReceiverType = std::conditional_t<std::is_same_v<StorageType, typename P::StorageType>, StorageType&, typename P::StorageType>;

	FieldProp(Annotation annotation, std::string name, StorageType MappedType::*member)
	    : Prop(std::move(annotation), std::weak_ptr<Prop>(), std::move(name))
	    , member(member) { }

	std::string name() const override {
		return P::name();
	}

	bool hasDefault() const override {
		return P::hasDefault();
	}

	void makeRequired() const override {
		if constexpr(!IsOptional<V>) {
			P::makeRequired();
		}
	};

	StorageType MappedType::*member;

   protected:
	void encodeTo_(Source& dst, MappedType const& value) const override {
		if constexpr(IsOptional<V>) {
			if(value.*this->member) {
				typename P::StorageType const v = *(value.*this->member);
				P::encodeTo(dst, v);
			}
		} else {
			if constexpr(std::is_same_v<StorageType, typename P::StorageType>) {
				P::encodeTo(dst, value.*this->member);
			} else {
				typename P::StorageType const v = value.*this->member;
				P::encodeTo(dst, v);
			}
		}
	}

	bool decodeFrom_(Source const& src, MappedType& value) const override {
		auto const next = src.next(this->ref);
		if(next == nullptr) {
			return false;
		}

		if constexpr(std::is_same_v<ReceiverType, StorageType>) {
			return P::decodeFrom(*next, value.*this->member);
		} else {
			std::remove_reference_t<ReceiverType> v;
			if(!P::decodeFrom(*next, v)) {
				return false;
			}

			value.*this->member = static_cast<StorageType>(v);
			return true;
		}
	}
};

template<typename V>
class StructuredProp: public CodecProp<V> {
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

	// TODO: using ordered map
	std::unordered_map<std::string, std::shared_ptr<CodecProp<StorageType>>> codecs;

	std::unordered_set<std::string> required_keys;

   protected:
	void encodeTo_(Source& dst, StorageType const& value) const override {
		for(auto const& [key, codec]: this->codecs) {
			codec->encodeTo(dst, value);
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

template<typename M, typename V>
inline auto field(Annotation annotation, std::string name, V M::*member) {
	using P = detail::FieldProp<M, V>;
	using D = detail::DescriberOf<P, detail::FieldContext<M>>;

	return D(std::make_shared<P>(std::move(annotation), std::move(name), member));
}

template<typename M, typename V>
inline auto field(std::string name, V M::*member) {
	return field(Annotation{}, std::move(name), member);
}

}  // namespace cray
