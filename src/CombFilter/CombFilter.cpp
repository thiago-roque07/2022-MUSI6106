#include <cassert>
#include <iostream>

#include "Util.h"
#include "Vector.h"

#include "CombFilter.h"

using namespace std;

CCombFilterBase::CCombFilterBase(int iBufferLengthInSamples)
{
    assert(iBufferLengthInSamples > 0);

    // allocate and init

    CCombFilterBase::filtbuff = new T[iBufferLengthInSamples];
}

template<class T>
CCombFilterBase<T>::CCombFilterBase(const CCombFilterBase& that)
{
}

CCombFilterBase::~CCombFilterBase()
{
    delete[] CCombFilterBase::filtbuff;
}

FilterFIR::~FilterFIR()
{
}

FilterIIR::~FilterIIR()
{
}
