#pragma once

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "CombFilterIf.h"


class CCombFilterBase
{
public:

    /*! feedforward or recursive comb filter */
    enum BaseCombFilterType_t
    {
        kCombFIR,           //!< finite impulse response filter
        kCombIIR,           //!< infinite impulse response filter

        kNumFilterTypes
    };

    /*! list of parameters for the comb filters */
    enum BaseFilterParam_t
    {
        kParamGain,         //!< gain as factor (usually -1...1)
        kParamDelay,        //!< delay in seconds for specification of comb width

        kNumFilterParams
    };

    CCombFilterBase(int iBufferLengthInSamples, int iNumChannels);
    virtual ~CCombFilterBase();

    Error_t setParam(BaseFilterParam_t eParam, float fParamValue);
    float   getParam(BaseFilterParam_t eParam);

    virtual Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) = 0;

protected:
    CRingBuffer<float>** m_ppCRingBuffer;

    int m_iBuffLength;              //!< MAX length of the internal buffer
    int m_iNumChannels;

    float ParamGain;         //!< gain as factor (usually -1...1)
    float ParamDelay;        //!< delay in SAMPLES for specification of comb width
    
    

private:
    CCombFilterBase(const CCombFilterBase& that);
	
};

class FilterIIR : public CCombFilterBase
{
public:
    FilterIIR(int iBufferLengthInSamples, int iNumChannels) : CCombFilterBase(iBufferLengthInSamples, iNumChannels) {};
    //virtual ~FilterIIR() {};

    Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) override;

private:
};

class FilterFIR : public CCombFilterBase
{
public:
    FilterFIR(int iBufferLengthInSamples, int iNumChannels) : CCombFilterBase(iBufferLengthInSamples, iNumChannels) {};
    //virtual ~FilterFIR() {};

    Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) override;

private:
};