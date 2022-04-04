
#include "FastConv.h"

CFastConv::CFastConv( void )
{
}

CFastConv::~CFastConv( void )
{
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/)
{

    return Error_t::kNoError;
}

Error_t CFastConv::reset()
{
    return Error_t::kNoError;
}

Error_t CFastConv::process (float* pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers )
{
    return Error_t::kNoError;
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer)
{
    return Error_t::kNoError;
}
