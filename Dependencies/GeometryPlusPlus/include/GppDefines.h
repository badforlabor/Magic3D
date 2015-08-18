#pragma once

#ifdef GPP_SOURCE
#define GPP_EXPORT
#else
#ifdef WIN32
#if GPP_BUILD
#define GPP_EXPORT __declspec(dllexport)
#else
#define GPP_EXPORT __declspec(dllimport)
#endif
#else
#define GPP_EXPORT
#endif
#endif

namespace GPP
{
    typedef double Real;
    typedef int Int;
    typedef unsigned int UInt;
    typedef long long LongInt;
    typedef unsigned long long ULongInt;
    static const Real REAL_TOL = 1.0e-15;
}
