#include <cassert>
#include <iostream>

#include "Util.h"
#include "Vector.h"

#include "CombFilter.h"

using namespace std;

CCombFilterBase::CCombFilterBase(int iBufferLengthInSamples, int iNumChannels) :
    m_ppCRingBuffer(0),
    m_iNumChannels(iNumChannels)
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

Error_t CCombFilterBase::setParam(BaseFilterParam_t eParam, float fParamValue)
{

    switch (eParam)
    {
    case CCombFilterIf::kParamGain:
        ParamGain = fParamValue;
        break;

    case CCombFilterIf::kParamDelay:

        ParamDelay = fParamValue;
        for (int m = 0; m < m_iNumChannels; m++) 
        {
            m_ppCRingBuffer[m]->reset();

            for (int i = 0; i < ParamDelay; i++) 
            {
                m_ppCRingBuffer[m]->putPostInc(0.f);
            }
        }
        break;
    }

    return Error_t();
}

float CCombFilterBase::getParam(BaseFilterParam_t eParam)
{
    switch (eParam)
    {
    case CCombFilterIf::kParamGain:
        return ParamGain;
        break;
    case CCombFilterIf::kParamDelay:
        return ParamDelay;
        break;
    }
    return 0.0f;
}



Error_t CCombFilterBase::process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames)
{
    return Error_t();
}

//FilterFIR::~FilterFIR()
//{
//}
//
//FilterIIR::~FilterIIR()
//{
//}

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

    return Error_t();
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

    return Error_t();
}
