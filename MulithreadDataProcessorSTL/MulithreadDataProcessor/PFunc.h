#ifndef PFUNC_H
#define PFUNC_H

template<typename Rtn, typename... Args>
struct FPHelper
{
	typedef Rtn(*Function)(Args...);
};

template<typename Rtn, typename O, typename... Args>
struct FPMHelper
{
	typedef Rtn(O::*Function)(Args...);
};

template<typename Rtn, typename... Args>
using PFunc = typename FPHelper<Rtn, Args...>::Function;

template<typename Rtn, typename O, typename... Args>
using PMFunc = typename FPMHelper<Rtn, O, Args...>::Function;

#endif