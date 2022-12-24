#pragma once

#include <memory>
#include <string>
#include <utility>

#include "cray/detail/prop.hpp"
#include "cray/detail/props/mono-map.hpp"
#include "cray/detail/props/structured.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class MapProp: public TransitiveProp {
   public:
	template<typename Ctx>
	class DescriberBase;

	template<typename Ctx>
	class Describer: public DescriberBase<Ctx> {
	   public:
		using ContextType = Ctx;

		using DescriberBase<Ctx>::DescriberBase;

		template<typename D, std::derived_from<Prop> Next = typename D::PropType>
		    requires(std::derived_from<Next, CodecProp<typename Next::StorageType>>)
		auto of(Annotation annotation, D describer) const {
			using P = MonoMapPropOf<Next>;

			auto const prev = this->prop_->prev.lock();
			assert(prev != nullptr);

			auto new_curr  = makeProp<P>(std::move(annotation), prev, this->prop_->ref);
			new_curr->next = getProp(std::move(describer));

			return DescriberOf<P, Ctx>(std::move(new_curr));
		}

		template<typename D>
		inline auto of(D describer) const {
			return this->of<D>(Annotation{}, std::move(describer));
		}

		template<Type T>
		inline auto of(Annotation annotation) const {
			return this->of(std::move(annotation), prop<T>());
		}

		template<Type T>
		inline auto of() const {
			return this->of(Annotation{}, prop<T>());
		}

		template<typename V>
		auto to(Annotation annotation) const {
			using P = detail::StructuredProp<V>;
			using D = detail::DescriberOf<P, Ctx>;

			auto const prev = this->prop_->prev.lock();
			assert(prev != nullptr);

			auto new_curr = makeProp<P>(std::move(annotation), prev, this->prop_->ref);
			return D(std::move(new_curr));
		}

		template<typename V>
		inline auto to() const {
			return this->to<V>(Annotation{});
		}
	};

	using TransitiveProp::TransitiveProp;
};

template<>
struct PropOf_<Type::Map> {
	using type = MapProp;
};

template<typename Ctx>
class MapProp::DescriberBase: public detail::Describer<MapProp> {
   public:
	using detail::Describer<MapProp>::Describer;
};

template<>
class MapProp::DescriberBase<GettableContext>: public detail::Describer<MapProp> {
   public:
	using detail::Describer<MapProp>::Describer;
};

}  // namespace detail
}  // namespace cray
