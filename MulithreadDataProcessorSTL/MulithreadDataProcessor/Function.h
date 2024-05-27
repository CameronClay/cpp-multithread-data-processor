#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdlib.h>
#include <functional>
#include <concepts>
#include <type_traits>

//Predeclaration of Function
template<typename Sig> class Function;

namespace FunctionUtils {
	//------------------Function Concepts------------------
	template <typename Callable>
	concept memfn = std::is_member_function_pointer_v<Callable>;

	template <typename Callable, typename... Args>
	concept invocable_memfn = std::is_member_function_pointer_v<Callable> && std::is_invocable_v<Callable, Args...>;

	template<class T, class U>
	concept not_same_as = !std::is_same_v<T, U>;
}

//Store function call with some arguments bound (including member functions)
//Speeds up std::function by wrapper function pointer in lambda expression and binding arguments within a lambda expression where applicable
//Eliminates need for std::bind (which is also quite slow on some platforms)
template<typename RT, typename... UnboundArgs>
class Function<RT(UnboundArgs...)>
{
public:
	using Action = std::function<RT(UnboundArgs...)>;

	//also serves as move and copy constructor via MakeFunc overloads
	template<typename Callable, typename... BoundArgs>
	Function(Callable&& callable, BoundArgs&&... args) requires FunctionUtils::not_same_as<Callable, Action>
		: action(MakeFunc(std::forward<Callable>(callable), std::forward<BoundArgs>(args)...))
	{}

	Function() = default;
	Function(Function&&) = default;
	Function(const Function&) = default;
	Function& operator=(Function&&) = default;
	Function& operator=(const Function&) = default;

	operator bool() const {
		return bool(action);
	}

	decltype(auto) operator()(UnboundArgs&&... args) const {
		return action(std::forward<UnboundArgs>(args)...);
	}

private:
	template<typename Callable, typename... BoundArgs>
	static Action MakeFunc(Callable&& callable, BoundArgs&&... args) requires std::invocable<Callable, BoundArgs..., UnboundArgs...> {
		return [callable = std::forward<Callable>(callable), ...args = std::forward<BoundArgs>(args)](UnboundArgs&&... rest) mutable -> decltype(auto) {
			return std::invoke(callable, std::forward<decltype(args)>(args)..., std::forward<UnboundArgs>(rest)...);
			//return func(std::forward<decltype(args)>(args)..., std::forward<UnboundArgs>(rest)...);
		};
	}

	template<typename Callable, typename O, typename... BoundArgs>
	static Action MakeFunc(Callable&& callable, O* o, BoundArgs&&... args) requires FunctionUtils::invocable_memfn<Callable, BoundArgs..., UnboundArgs...> {
		return [o, callable = std::forward<Callable>(callable), ...args = std::forward<BoundArgs>(args)](UnboundArgs&&... rest) mutable -> decltype(auto) {
			return std::invoke(callable, *o, std::forward<decltype(args)>(args)..., std::forward<UnboundArgs>(rest)...);
			//return (o->*func)(std::forward<decltype(args)>(args)..., std::forward<UnboundArgs>(rest)...);
		};
	}

	template<typename Callable, typename O, typename... BoundArgs>
	static Action MakeFunc(Callable&& callable, O& o, BoundArgs&&... args) requires FunctionUtils::invocable_memfn<Callable, BoundArgs..., UnboundArgs...> {
		return [&o, callable = std::forward<Callable>(callable), ...args = std::forward<BoundArgs>(args)](UnboundArgs&&... rest) mutable -> decltype(auto) {
			return std::invoke(callable, o, std::forward<decltype(args)>(args)..., std::forward<UnboundArgs>(rest)...);
			//return (o.*func)(std::forward<decltype(args)>(args)..., std::forward<UnboundArgs>(rest)...);
		};
	}

	Action action;
};

#endif