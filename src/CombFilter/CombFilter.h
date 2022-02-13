#pragma once



class CCombFilterBase
{
public:
    CCombFilterBase(int iBufferLengthInSamples);
    virtual ~CCombFilterBase();

protected:


private:
    CCombFilterBase();
    CCombFilterBase(const CCombFilterBase& that);

    int m_iBuffLength;              //!< length of the internal buffer


	
};

class FilterIIR : public CCombFilterBase
{
public:
    FilterIIR(int iBufferLengthInSamples) :CCombFilterBase(iBufferLengthInSamples) {};;
    ~FilterIIR() {};


private:
};

class FilterFIR : public CCombFilterBase
{
public:
    FilterFIR(int iBufferLengthInSamples):CCombFilterBase(iBufferLengthInSamples) {};
    ~FilterFIR() {};

private:
};