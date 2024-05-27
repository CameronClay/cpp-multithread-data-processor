#ifndef FUNCTION_TRAITS_H
#define FUNCTION_TRAITS_H

#include <cstddef>

namespace ftraits {
	template<typename Callable>
	using call_op_t = decltype(&Callable::operator());

	//extract argument types from Callable
	template<typename Callable>
	struct fextract_args;

	//functor specialization
	template<typename Callable>
	struct fextract_args {
		using arg1 = typename fextract_args<call_op_t<Callable>>::arg1;
		using arg2 = typename fextract_args<call_op_t<Callable>>::arg2;
	};

	//function pointer specialization
	template<typename R, typename Arg1, typename Arg2>
	struct fextract_args<R(*)(Arg1, Arg2)> {
		using arg1 = Arg1;
		using arg2 = Arg2;
	};

	//member function specialization
	template<typename R, typename C, typename Arg1, typename Arg2>
	struct fextract_args<R(C::*)(Arg1, Arg2)> {
		using arg1 = Arg1;
		using arg2 = Arg2;
	};

	//const member function specialization
	template<typename R, typename C, typename Arg1, typename Arg2>
	struct fextract_args<R(C::*)(Arg1, Arg2) const> {
		using arg1 = Arg1;
		using arg2 = Arg2;
	};

	template<typename Callable>
	using fextract_arg1 = typename fextract_args<Callable>::arg1;

	template<typename Callable>
	using fextract_arg2 = typename fextract_args<Callable>::arg2;


	////functor specialization
	//template<typename Callable>
	//struct ExtractArgumentHelper {
	//	using type = typename ExtractArgumentHelper<decltype(&Callable::operator())>::type;
	//};
	//
	////function pointer specialization
	//template<typename R, typename Arg>
	//struct ExtractArgumentHelper<R(*)(std::size_t, Arg)> {
	//	using type = Arg;
	//};
	//
	////member function specialization
	//template<typename R, typename C, typename Arg>
	//struct ExtractArgumentHelper<R(C::*)(std::size_t, Arg)> {
	//	using type = Arg;
	//};
	//
	////const member function specialization
	//template<typename R, typename C, typename Arg>
	//struct ExtractArgumentHelper<R(C::*)(std::size_t, Arg) const> {
	//	using type = Arg;
	//};

	////template with signature of functor (specialization not needed)
	//template<template<typename> typename Functor, typename R, typename Arg>
	//struct ExtractArgumentHelper<Functor<R(std::size_t, Arg)>> {
	//	using type = Arg;
	//};

	//template<typename Callable>
	//using ExtractArgumentHelper_t = typename ExtractArgumentHelper<Callable>::type;
	//
	//template<typename T>
	//using CallOperator_t = decltype(&T::operator());
	//
	//template<typename T, typename = std::void_t<>>
	//inline constexpr bool HasCallOperator_v = false;
	//
	//template<typename T>
	//inline constexpr bool HasCallOperator_v<T, std::void_t<CallOperator_t<T>>> = true;
	//
	//template<typename Callable, bool b>
	//struct ExtractArgumentCond;
	//
	//template<typename Callable>
	//struct ExtractArgumentCond<Callable, false> {
	//	using type = ExtractArgumentHelper_t<Callable>;
	//};
	//
	//template<typename Callable>
	//struct ExtractArgumentCond<Callable, true> {
	//	using type = ExtractArgumentHelper_t<CallOperator_t<Callable>>;
	//};

	//template<typename Callable>
	//using ExtractArgument = ExtractArgumentCond<Callable, HasCallOperator_v<Callable>>;
	//
	//template<typename Callable>
	//using ExtractArgument_t = typename ExtractArgument<Callable>::type;
}

#endif