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

	template<typename Callable, typename... BoundArgs>
	Function(Callable&& callable, BoundArgs&&... args) requires std::invocable<Callable, BoundArgs..., UnboundArgs...>
		:
		action(
			[callable = std::forward<Callable>(callable), ...boundArgs = std::forward<BoundArgs>(args)](auto&&... unboundArgs) mutable -> decltype(auto) {
				return std::invoke(callable, std::forward<decltype(boundArgs)>(boundArgs)..., std::forward<decltype(unboundArgs)>(unboundArgs)...);
			}
		)
	{}

	Function(Function&&) = default;
	Function(const Function&) = default;
	Function& operator=(Function&&) = default;
	Function& operator=(const Function&) = default;

	operator bool() const {
		return bool(action);
	}

	////meed this separate variadic template for perfect forwarding (because perfect forwarding requires a function template parameter)
	////need concept constrait because otherwise number of args in UBArgs... may not match that of UnboundArgs... could also check length instead
	//template<typename... UBArgs>
	//decltype(auto) operator()(UBArgs&&... args) const requires std::invocable<Action, UBArgs...> {
	//	return action(std::forward<UBArgs>(args)...);
	//}

	decltype(auto) operator()(UnboundArgs... args) const {
		return action(args...);
	}

private:
	Action action;
};