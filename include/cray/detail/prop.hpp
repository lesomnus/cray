#pragma once

#include <cassert>
#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "cray/source.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {

struct GetterQuery {
};

struct OptGetterQuery { };

class Prop {
   public:
	Prop() { }

	Prop(Annotation annotation)
	    : annotation(std::move(annotation)) { }

	/**
	 * @brief Construct a new Prop object. Note that \a source be nullptr since it is set by the
	 * receiver who takes the describer.
	 * 
	 * @param annotation 
	 * @param prev 
	 * @param ref 
	 */
	Prop(Annotation annotation, std::weak_ptr<Prop> prev, Reference ref)
	    : annotation(std::move(annotation))
	    , prev(std::move(prev))
	    , ref(std::move(ref)) { }

	virtual ~Prop() { }

	virtual Type type() const {
		return Type::Unspecified;
	}

	virtual std::string name() const {
		return "unknown";
	}

	virtual bool ok() const = 0;

	virtual bool hasDefault() const = 0;

	virtual void markRequired(Reference const& ref) {
		throw InvalidAccessError();
	};

	virtual bool needs(Reference const& ref) const {
		throw InvalidAccessError();
	}

	virtual void makeRequired() const {
		auto const prev = this->prev.lock();
		if(prev == nullptr) [[unlikely]] {
			return;
		}

		prev->markRequired(this->ref);
	}

	inline bool isNeeded() const {
		auto const prev = this->prev.lock();
		if(prev == nullptr) [[unlikely]] {
			return false;
		}

		return prev->needs(this->ref);
	}

	virtual void assign(Reference const& ref, std::shared_ptr<Prop> prop) {
		throw InvalidAccessError();
	}

	virtual std::shared_ptr<Prop> at(Reference const& ref) const {
		throw InvalidAccessError();
	}

	Annotation              annotation;
	std::shared_ptr<Source> source;
	std::weak_ptr<Prop>     prev;
	Reference               ref;
};

class TransitiveProp: public virtual Prop {
   public:
	using Prop::Prop;

	Type type() const override {
		throw InvalidAccessError();
	}

	std::string name() const override {
		throw InvalidAccessError();
	}

	bool ok() const {
		throw InvalidAccessError();
	}

	bool hasDefault() const {
		throw InvalidAccessError();
	}
};

template<typename V>
class CodecProp: public virtual Prop {
   public:
	using StorageType = V;

	using Prop::Prop;

	virtual std::string name() const {
		return "unnamed Codec";
	}

	bool hasDefault() const override {
		return default_value.has_value();
	}

	inline void encodeTo(Source& dst, StorageType const& value) const {
		this->encodeTo_(dst, value);
	}

	inline bool decodeFrom(Source const& src, StorageType& value) const {
		if(this->decodeFrom_(src, value)) {
			return true;
		}

		if(this->default_value.has_value()) {
			value = this->default_value.value();
			return true;
		}

		return false;
	}

	inline void encode(StorageType const& value) const {
		this->encodeTo(*this->source, value);
	}

	inline bool decode(StorageType& value) const {
		return this->decodeFrom(*this->source, value);
	}

	std::optional<StorageType> opt() const {
		StorageType value;
		if(this->decode(value)) {
			return value;
		} else {
			return std::nullopt;
		}
	}

	inline StorageType get() const {
		this->makeRequired();
		return this->opt().value_or(StorageType());
	}

	std::optional<StorageType> default_value;

   protected:
	virtual void encodeTo_(Source& dst, StorageType const& value) const = 0;

	virtual bool decodeFrom_(Source const& src, StorageType& value) const = 0;
};

class RootProp: public Prop {
   public:
	RootProp(std::shared_ptr<Source> source)
	    : Prop(Annotation{}, std::weak_ptr<Prop>(), Reference()) {
		this->source = std::move(source);
	}

	std::string name() const override {
		return "Root";
	}

	bool ok() const override {
		if(this->next == nullptr) [[unlikely]] {
			return false;
		}

		return this->next->ok();
	}

	bool hasDefault() const override {
		return false;
	}

	void markRequired(Reference const& ref) override {
		this->next_is_required = true;
	};

	bool needs(Reference const& ref) const override {
		return this->next_is_required;
	}

	void assign(Reference const& ref, std::shared_ptr<Prop> prop) override {
		prop->source           = this->source;
		this->next             = std::move(prop);
		this->next_is_required = false;
	}

	std::shared_ptr<Prop> at(Reference const& ref) const override {
		return this->next;
	}

	std::shared_ptr<Prop> next;
	bool                  next_is_required;
};

template<Type T>
struct PropOf_ { };

template<Type T>
using PropOf = PropOf_<T>::type;

template<typename T>
struct PropFor_;

template<typename T>
struct PropFor_<std::optional<T>> {
	using type = PropOf<TypeFor<T>>;
};

template<typename T>
using PropFor = PropFor_<T>::type;

struct EmptyContext { };
struct GettableContext { };

template<typename T, typename Ctx>
concept WithContext = std::same_as<typename T::ContextType, Ctx>;

template<typename T>
struct DescriberPropGetter;

template<std::derived_from<Prop> P>
class Describer {
   public:
	using PropType = P;

	Describer(std::shared_ptr<P> prop)
	    : prop_(std::move(prop)) { }

   protected:
	template<typename T>
	friend struct DescriberPropGetter;

	std::shared_ptr<P> prop_;
};

template<typename T>
struct DescriberPropGetter {
	auto get() && {
		return std::move(this->value.prop_);
	}

	T value;
};

template<std::derived_from<Prop> P>
constexpr auto getProp(Describer<P> const& describer) {
	return DescriberPropGetter<Describer<P>>{describer}.get();
}

template<std::derived_from<Prop> P>
constexpr auto getProp(Describer<P>&& describer) {
	return DescriberPropGetter<Describer<P>>{std::move(describer)}.get();
}

template<std::derived_from<Prop> P, typename Ctx>
using DescriberOf = typename P::template Describer<Ctx>;

/**
 * @brief Create a Prop and connect to it with \a prev by \a ref.
 * 
 * @tparam P Prop to create.
 * @param annotation 
 * @param prev Previous Prop to be connected.
 * @param ref Reference from previous Prop.
 * @return std::shared_ptr<P> 
 */
template<std::derived_from<Prop> P>
std::shared_ptr<P> makeProp(Annotation annotation, std::shared_ptr<Prop> const& prev, Reference ref) {
	assert(prev != nullptr);

	auto curr = std::make_shared<P>(std::move(annotation), prev, std::move(ref));
	prev->assign(curr->ref, curr);

	return curr;
}

}  // namespace detail

template<Type T>
auto prop(Annotation annotation) {
	using P = detail::PropOf<T>;
	using D = P::template Describer<detail::EmptyContext>;

	return D(std::make_shared<P>(std::move(annotation), std::weak_ptr<detail::Prop>(), Reference()));
}

template<Type T>
auto prop() {
	return prop<T>(Annotation{});
}

static constexpr detail::GetterQuery    get{};
static constexpr detail::OptGetterQuery opt{};

}  // namespace cray
