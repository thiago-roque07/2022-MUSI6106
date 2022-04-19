
#if !defined(__FastConv_HEADER_INCLUDED__)
#define __FastConv_HEADER_INCLUDED__

#pragma once

#include "ErrorDef.h"
#include "Fft.h"

/*! \brief interface for fast convolution
*/
class CFastConv
{
public:
    enum ConvCompMode_t
    {
        kTimeDomain,
        kFreqDomain,

        kNumConvCompModes
    };

    CFastConv(void);
    virtual ~CFastConv(void);

    /*! initializes the class with the impulse response and the block length
    \param pfImpulseResponse impulse response samples (mono only)
    \param iLengthOfIr length of impulse response
    \param iBlockLength processing block size
    \return Error_t
    */
    Error_t init(float* pfImpulseResponse, int iLengthOfIr, int iBlockLength = 8192, ConvCompMode_t eCompMode = kFreqDomain);

    /*! resets all internal class members
    \return Error_t
    */
    Error_t reset ();

    /*! computes the output with reverb
    \param pfOutputBuffer (mono)
    \param pfInputBuffer (mono)
    \param iLengthOfBuffers can be anything from 1 sample to 10000000 samples
    \return Error_t
    */
    Error_t process(float* pfOutputBuffer, const float* pfInputBuffer, int iLengthOfBuffers);

    /*! return the 'tail' after processing has finished (identical to feeding in zeros
    \param pfOutputBuffer (mono)
    \return Error_t
    */
    Error_t flushBuffer(float* pfOutputBuffer);

private:
    ConvCompMode_t m_ConvType;
    int m_iLengthOfIr;
    float *m_pfImpulseResponse;
    float *m_pfTail;
    
    float* m_pfBlockBuffer;
    int m_iblockSize;


    float *m_pfTimeInput;
    float *m_pfTimeIr;
    float *m_pfTmpConv;
    CFft::complex_t *m_pfFreqInput;
    CFft::complex_t *m_pfFreqIr;
    CFft::complex_t *m_pfFreqConv;
    float *m_pfRealInput;
    float *m_pfImagInput;
    float *m_pfRealIr;
    float *m_pfImagIr;
    float *m_pfRealConv;
    float *m_pfImagConv;

    float *m_pfTimeConv;

    int m_iFftLength;

    CFft* m_pCFftInstance;
};


#endif
