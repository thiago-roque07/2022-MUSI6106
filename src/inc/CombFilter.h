#pragma once

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "CombFilterIf.h"


class CCombFilterBase
{
public:

    CCombFilterBase(int iBufferLengthInSamples, int iNumChannels);
    virtual ~CCombFilterBase();

    Error_t setGain(float fParamValue);
    Error_t setDelay(float fParamValue);
    Error_t reset();

    float   getGain();
    float   getDelay();

    virtual Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) = 0;

protected:
    CRingBuffer<float>** m_ppCRingBuffer;

    int m_iBuffLength;              //!< MAX length of the internal buffer
    int m_iNumChannels;

    float ParamGain;         //!< gain as factor (usually -1...1)
    float ParamDelay;        //!< delay in SAMPLES for specification of comb width

    bool isInitialized;
    bool firstRound;
 
private:
    CCombFilterBase(const CCombFilterBase& that);
};

class FilterIIR : public CCombFilterBase
{
public:
    FilterIIR(int iBufferLengthInSamples, int iNumChannels) : CCombFilterBase(iBufferLengthInSamples, iNumChannels) {};

    Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) override;

private:
};

class FilterFIR : public CCombFilterBase
{
public:
    FilterFIR(int iBufferLengthInSamples, int iNumChannels) : CCombFilterBase(iBufferLengthInSamples, iNumChannels) {};

    Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) override;

private:
};