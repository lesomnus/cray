#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include "cray/detail/prop.hpp"
#include "cray/detail/props/array.hpp"
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

		template<std::size_t N, typename D, std::derived_from<Prop> Next = typename D::PropType>
		    requires(std::derived_from<Next, CodecProp<typename Next::StorageType>>)
		auto of(Annotation annotation, D describer) const {
			using P = ArrayPropOf<Next, N>;
			return this->replaceWith_<P, D>(std::move(annotation), std::move(describer));
		}

		template<std::size_t N, typename D>
		inline auto of(D describer) const {
			return this->of<N, D>(Annotation{}, std::move(describer));
		}

		template<Type T, std::size_t N>
		    requires IsScalarType<T>
		inline auto of(Annotation annotation) const {
			return this->of<N>(std::move(annotation), prop<T>());
		}

		template<Type T, std::size_t N>
		    requires IsScalarType<T>
		inline auto of() const {
			return this->of<N>(Annotation{}, prop<T>());
		}

		template<typename D, std::derived_from<Prop> Next = typename D::PropType>
		    requires(std::derived_from<Next, CodecProp<typename Next::StorageType>>)
		auto of(Annotation annotation, D describer) const {
			using P = MonoListPropOf<Next>;
			return this->replaceWith_<P, D>(std::move(annotation), std::move(describer));
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

	   private:
		template<typename P, typename D>
		auto replaceWith_(Annotation annotation, D describer) const {
			auto prev_prop = this->prop_->prev.lock();
			auto new_curr  = makeProp<P>(std::move(annotation), std::move(prev_prop), this->prop_->ref);

			auto next_prop      = getProp(std::move(describer));
			next_prop->prev     = new_curr;
			new_curr->next_prop = std::move(next_prop);

			return DescriberOf<P, Ctx>(std::move(new_curr));
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
