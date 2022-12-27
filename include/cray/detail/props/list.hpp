#pragma once

#include "cray/detail/prop.hpp"
#include "cray/detail/props/mono-list.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class ListProp: public TransitiveProp {
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
			using P = MonoListPropOf<Next>;

			auto prev     = this->prop_->prev.lock();
			auto new_curr = makeProp<P>(std::move(annotation), std::move(prev), this->prop_->ref);

			auto next_prop      = getProp(std::move(describer));
			next_prop->prev     = new_curr;
			new_curr->next_prop = std::move(next_prop);

			return DescriberOf<P, Ctx>(std::move(new_curr));
		}

		template<typename D>
		inline auto of(D describer) const {
			return this->of<D>(Annotation{}, std::move(describer));
		}

		template<Type T>
		    requires IsScalarType<T>
		inline auto of(Annotation annotation) const {
			return this->of(std::move(annotation), prop<T>());
		}

		template<Type T>
		    requires IsScalarType<T>
		inline auto of() const {
			return this->of(Annotation{}, prop<T>());
		}
	};

	using TransitiveProp::TransitiveProp;
};

template<>
struct PropOf_<Type::List> {
	using type = ListProp;
};

template<typename Ctx>
class ListProp::DescriberBase: public detail::Describer<ListProp> {
   public:
	using detail::Describer<ListProp>::Describer;
};

template<>
class ListProp::DescriberBase<GettableContext>: public detail::Describer<ListProp> {
   public:
	using detail::Describer<ListProp>::Describer;
};

}  // namespace detail
}  // namespace cray
