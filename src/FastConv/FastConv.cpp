
#include "FastConv.h"

CFastConv::CFastConv( void )
{
}

CFastConv::~CFastConv( void )
{
    delete[] m_pfImpulseResponse;
    delete[] m_pfTail;
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/)
{
    m_ConvType = eCompMode;
    m_pfImpulseResponse = new float [iLengthOfIr];
    m_pfTail = new float [iLengthOfIr-1];
    m_iLengthOfIr = iLengthOfIr;

    for (int i = 0; i < iLengthOfIr; i++)
    {
        m_pfImpulseResponse[i] = pfImpulseResponse[i];
    }

    return Error_t::kNoError;
}

Error_t CFastConv::reset()
{
    return Error_t::kNoError;
}

Error_t CFastConv::process (float* pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers )
{
    if (m_ConvType == CFastConv::ConvCompMode_t::kTimeDomain)
    {
        for (int i = 0; i < iLengthOfBuffers; i++) {
            pfOutputBuffer[i] = 0;
            for (int j = 0; j < m_iLengthOfIr; j++) {
                if (i - j < 0) break;
                pfOutputBuffer[i] += pfInputBuffer[i - j] * m_pfImpulseResponse[j];
            }
        }
        for (int i = iLengthOfBuffers; i < iLengthOfBuffers + m_iLengthOfIr - 1; i++) {
            m_pfTail[i - iLengthOfBuffers] = 0;
            for (int j = 0; j < m_iLengthOfIr; j++) {
                if (i - j >= iLengthOfBuffers) break;
                m_pfTail[i - iLengthOfBuffers] += pfInputBuffer[i - j] * m_pfImpulseResponse[j];
            }
        }
    }

    return Error_t::kNoError;
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer)
{
    for (int i = 0; i < m_iLengthOfIr-1; i++)
    {
        pfOutputBuffer[i] = m_pfTail[i];
    }

    return Error_t::kNoError;
}
