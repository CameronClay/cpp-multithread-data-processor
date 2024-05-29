export module Function;

import <cstdlib>;
import <functional>;
import <concepts>;
import <type_traits>;

//Predeclaration of Function
export template<typename Sig> class Function;

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
export template<typename RT, typename... UnboundArgs>
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

	//meed this separate variadic template for perfect forwarding (because perfect forwarding requires a function template parameter)
	template<typename... UBArgs>
	decltype(auto) operator()(UBArgs&&... args) const {
		return action(std::forward<UnboundArgs>(args)...);
	}

private:
	template<typename Callable, typename... BoundArgs>
	static Action MakeFunc(Callable&& callable, BoundArgs&&... boundArgs) requires std::invocable<Callable, BoundArgs..., UnboundArgs...> {
		return [callable = std::forward<Callable>(callable), ...args = std::forward<BoundArgs>(boundArgs)](auto&&... unboundArgs) mutable -> decltype(auto) {
			return std::invoke(callable, std::forward<decltype(args)>(args)..., std::forward<decltype(unboundArgs)>(unboundArgs)...);
			//return func(std::forward<decltype(args)>(boundArgs)..., std::forward<UnboundArgs>(unboundArgs)...);
		};
	}

	template<typename Callable, typename O, typename... BoundArgs>
	static Action MakeFunc(Callable&& callable, O* o, BoundArgs&&... boundArgs) requires FunctionUtils::invocable_memfn<Callable, BoundArgs..., UnboundArgs...> {
		return [o, callable = std::forward<Callable>(callable), ...args = std::forward<BoundArgs>(boundArgs)](auto&&... unboundArgs) mutable -> decltype(auto) {
			return std::invoke(callable, *o, std::forward<decltype(args)>(args)..., std::forward<decltype(unboundArgs)>(unboundArgs)...);
			//return (o->*func)(std::forward<decltype(args)>(boundArgs)..., std::forward<UnboundArgs>(unboundArgs)...);
		};
	}

	template<typename Callable, typename O, typename... BoundArgs>
	static Action MakeFunc(Callable&& callable, O& o, BoundArgs&&... boundArgs) requires FunctionUtils::invocable_memfn<Callable, BoundArgs..., UnboundArgs...> {
		return [&o, callable = std::forward<Callable>(callable), ...args = std::forward<BoundArgs>(boundArgs)](auto&&... unboundArgs) mutable -> decltype(auto) {
			return std::invoke(callable, o, std::forward<decltype(args)>(args)..., std::forward<decltype(unboundArgs)>(unboundArgs)...);
			//return (o.*func)(std::forward<decltype(args)>(boundArgs)..., std::forward<UnboundArgs>(unboundArgs)...);
		};
	}

	Action action;
};