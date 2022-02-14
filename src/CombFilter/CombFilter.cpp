#include <cassert>
#include <iostream>

#include "Util.h"
#include "Vector.h"

#include "CombFilter.h"


using namespace std;

CCombFilterBase::CCombFilterBase(int iBufferLengthInSamples, int iNumChannels) :
    m_ppCRingBuffer(0),
    m_iNumChannels(iNumChannels),
    ParamGain(0),
    ParamDelay(0),
    m_iBuffLength(0),
    isInitialized(false)
{
    assert(iBufferLengthInSamples > 0);
    assert(iNumChannels > 0);

    m_ppCRingBuffer = new CRingBuffer<float>*[m_iNumChannels];
    for (int i = 0; i < m_iNumChannels; i++) {
        m_ppCRingBuffer[i] = new CRingBuffer<float>(iBufferLengthInSamples);
    }

}

CCombFilterBase::~CCombFilterBase()
{
    for (int i = 0; i < m_iNumChannels; i++) {
        delete m_ppCRingBuffer[i];
    }
    delete[] m_ppCRingBuffer;
}

Error_t CCombFilterBase::reset()
{
    for (int c = 0; c < m_iNumChannels; c++)
    {
        m_ppCRingBuffer[c]->reset();
    }
    return Error_t::kNoError;
}

Error_t CCombFilterBase::setGain(float fParamValue)
{
    ParamGain = fParamValue;
    return Error_t::kNoError;
}

float CCombFilterBase::getGain()
{
    return ParamGain;
}

Error_t CCombFilterBase::setDelay(float fParamValue)
{
    ParamDelay = fParamValue;
    return Error_t::kNoError;
}

float CCombFilterBase::getDelay()
{
    return ParamDelay;
}

Error_t FilterIIR::process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames)
{
    float coeff = ParamGain;

    for (int nChan = 0; nChan < m_iNumChannels; nChan++)
    {
        for (int i = 0; i < iNumberOfFrames; i++)
        {
            ppfOutputBuffer[nChan][i] = ppfInputBuffer[nChan][i] + coeff * m_ppCRingBuffer[nChan]->getPostInc();
            m_ppCRingBuffer[nChan]->putPostInc(ppfOutputBuffer[nChan][i]);
        }
    }
    return Error_t::kNoError;
}

Error_t FilterFIR::process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames)
{

    float coeff = ParamGain;

    for (int nChan = 0; nChan < m_iNumChannels; nChan++)
    {
        for (int i = 0; i < iNumberOfFrames; i++)
        {
            ppfOutputBuffer[nChan][i] = ppfInputBuffer[nChan][i] + coeff * m_ppCRingBuffer[nChan]->getPostInc();
            m_ppCRingBuffer[nChan]->putPostInc(ppfInputBuffer[nChan][i]);
        }
    }
    return Error_t::kNoError;

}
