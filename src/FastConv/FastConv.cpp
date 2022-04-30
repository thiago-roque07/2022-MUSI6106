
#include "FastConv.h"

CFastConv::CFastConv() :
    m_pCConv(0),
    m_ConvType(kNumConvCompModes)
{
}

CFastConv::~CFastConv()
{
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/)
{
    if (eCompMode == kTimeDomain)
    {
        m_pCConv = static_cast<CBaseConv*>(new CTimeConv());
    }
    else if (eCompMode == kFreqDomain)
    {
        m_pCConv = static_cast<CBaseConv*> (new CFftConv());
    }

    return m_pCConv->initConv(pfImpulseResponse, iLengthOfIr, iBlockLength);
    //return Error_t::kNoError;
}


Error_t CFastConv::reset()
{
    if (m_pCConv) delete m_pCConv;
    m_pCConv = 0;
    return Error_t::kNoError;
}

Error_t CFastConv::process (float* pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers )
{
    return m_pCConv->processConv(pfOutputBuffer, pfInputBuffer, iLengthOfBuffers);
    //return Error_t::kNoError;;
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer)
{
    return m_pCConv->flushBufferConv(pfOutputBuffer);
    //return Error_t::kNoError;;
}


/**********************************************/

/*
 * Constructor for the Time Domain Convolution Class
 */
CTimeConv::CTimeConv()
{
}

/*
 * Destructor for the Time Domain Convolution Class
 */
CTimeConv::~CTimeConv()
{
    resetConv();
}

Error_t CTimeConv::initConv(float* pfImpulseResponse, int iLengthOfIr, int iBlockLength)
{
    if (iLengthOfIr < 1) {
        return Error_t::kFunctionInvalidArgsError;
    }

    m_pfImpulseResponse = new float[iLengthOfIr];

    m_pfInputTail = new float[(2 * iLengthOfIr) - 1];
    m_pfBlockBuffer = new float[iBlockLength];

    m_iLengthOfIr = iLengthOfIr;
    m_iblockSize = iBlockLength;


    for (int i = 0; i < iLengthOfIr; i++)
    {
        m_pfImpulseResponse[i] = pfImpulseResponse[i];
    }

    return Error_t::kNoError;
}

Error_t CTimeConv::resetConv()
{
    delete[] m_pfImpulseResponse;
    m_pfImpulseResponse = 0;
    delete[] m_pfInputTail;
    m_pfInputTail = 0;
    delete[] m_pfBlockBuffer;
    m_pfBlockBuffer = 0;

    return Error_t::kNoError;
}

Error_t CTimeConv::processConv(float* pfOutputBuffer, const float* pfInputBuffer, int iLengthOfBuffers)
{
    if (m_iLengthOfIr < iLengthOfBuffers)
    {
        m_iTailIndex = iLengthOfBuffers - m_iLengthOfIr + 1;
        for (int i = m_iTailIndex; i < iLengthOfBuffers + m_iLengthOfIr - 1; i++)
        {
            m_pfInputTail[i - m_iTailIndex] = (i < iLengthOfBuffers) ? pfInputBuffer[i] : 0;
        }
    }
    else
    {
        m_iTailIndex = iLengthOfBuffers;
        for (int i = 0; i < iLengthOfBuffers + m_iLengthOfIr - 1; i++)
        {
            m_pfInputTail[i] = (i < iLengthOfBuffers) ? pfInputBuffer[i] : 0;
        }
    }

    for (int i = 0; i < iLengthOfBuffers; i++) 
    {
        pfOutputBuffer[i] = 0;
        for (int j = 0; j < m_iLengthOfIr; j++) {
            if (i - j < 0) break;
            pfOutputBuffer[i] += pfInputBuffer[i - j] * m_pfImpulseResponse[j];
        }
    }
    return Error_t::kNoError;
}

Error_t CBaseConv::flushBufferConv(float* pfOutputBuffer)
{
    for (int i = m_iTailIndex; i < m_iTailIndex + m_iLengthOfIr - 1; i++) {
        pfOutputBuffer[i - m_iTailIndex] = 0;
        for (int j = 0; j < m_iLengthOfIr; j++) {
            if (i - j < 0) break;
            pfOutputBuffer[i - m_iTailIndex] += m_pfInputTail[i - j] * m_pfImpulseResponse[j];
        }
    }
    return Error_t::kNoError;
}


/*
 * Constructor for the FFT Convolution Class
 */
CFftConv::CFftConv()
{
}

/*
 * Destructor for the FFT Convolution Class
 */
CFftConv::~CFftConv()
{
    resetConv();
}

/*
 * Initialization function for the FFT Convolution Class
 */
Error_t CFftConv::initConv(float* pfImpulseResponse, int iLengthOfIr, int iBlockLength)
{
    if (iLengthOfIr < 1) {
        return Error_t::kFunctionInvalidArgsError;
    }

    m_pfImpulseResponse = new float[iLengthOfIr];

    m_pfInputTail = new float[(2 * iLengthOfIr) - 1];
    m_pfBlockBuffer = new float[iBlockLength];

    m_iLengthOfIr = iLengthOfIr;
    m_iblockSize = iBlockLength;

    CFft::createInstance(m_pCFftInstance);
    m_pCFftInstance->initInstance(2 * m_iblockSize, 1, CFft::kWindowHann, CFft::CFft::kPreWindow);

    for (int i = 0; i < iLengthOfIr; i++)
    {
        m_pfImpulseResponse[i] = pfImpulseResponse[i];
    }

    m_bLongBlock = false;
    return Error_t::kNoError;
}

/*
 * Reset function for the FFT Convolution Class
 */
Error_t CFftConv::resetConv()
{
    delete[] m_pfImpulseResponse;
    m_pfImpulseResponse = 0;
    delete[] m_pfInputTail;
    m_pfInputTail = 0;
    delete[] m_pfBlockBuffer;
    m_pfBlockBuffer = 0;

    m_bLongBlock = false;

    m_pCFftInstance->resetInstance();
    CFft::destroyInstance(m_pCFftInstance);

    return Error_t::kNoError;
}

/*
 * Process function for the FFT Convolution Class
 */
Error_t CFftConv::processConv(float* pfOutputBuffer, const float* pfInputBuffer, int iLengthOfBuffers)
{
    if (m_iLengthOfIr < iLengthOfBuffers)
    {
        m_iTailIndex = iLengthOfBuffers - m_iLengthOfIr + 1;
        for (int i = m_iTailIndex; i < iLengthOfBuffers + m_iLengthOfIr - 1; i++)
        {
            m_pfInputTail[i - m_iTailIndex] = (i < iLengthOfBuffers) ? pfInputBuffer[i] : 0;
        }
    }
    else
    {
        m_iTailIndex = iLengthOfBuffers;
        for (int i = 0; i < iLengthOfBuffers + m_iLengthOfIr - 1; i++)
        {
            m_pfInputTail[i] = (i < iLengthOfBuffers) ? pfInputBuffer[i] : 0;
        }
    }
    m_bLongBlock = (m_iblockSize > iLengthOfBuffers) ? true : false;

    int fftBlockSize = 2 * m_iblockSize;

    m_pfFreqInput = new CFft::complex_t[fftBlockSize];
    m_pfFreqIr = new CFft::complex_t[fftBlockSize];
    m_pfFreqConv = new CFft::complex_t[fftBlockSize];
    m_pfTimeInput = new float[fftBlockSize];
    m_pfTimeIr = new float[fftBlockSize];
    m_pfRealInput = new float[fftBlockSize];
    m_pfImagInput = new float[fftBlockSize];
    m_pfRealIr = new float[fftBlockSize];
    m_pfImagIr = new float[fftBlockSize];
    m_pfRealConv = new float[fftBlockSize];
    m_pfImagConv = new float[fftBlockSize];
    m_pfTmpConv = new float[fftBlockSize];

    for (int i = 0; i < iLengthOfBuffers; i++)
    {
        pfOutputBuffer[i] = 0;
    }

    int nBlockInput = 0;
    while ((static_cast<float>(nBlockInput) < (static_cast<float>(iLengthOfBuffers) / static_cast<float>(m_iblockSize))) || (nBlockInput == 0))
    {
        for (int i = m_iblockSize * nBlockInput; i < m_iblockSize * (nBlockInput + 2); i++)
        {
            if (nBlockInput == 0) m_pfTimeInput[i] = (i < m_iblockSize || (i - m_iblockSize) > iLengthOfBuffers - 1) ? 0 : pfInputBuffer[i - m_iblockSize] * fftBlockSize;
            else m_pfTimeInput[i - m_iblockSize * nBlockInput] = (i < m_iblockSize* (nBlockInput + 1) && i < iLengthOfBuffers) ? pfInputBuffer[i] * fftBlockSize : 0;
        }
        m_pCFftInstance->doFft(m_pfFreqInput, m_pfTimeInput);
        m_pCFftInstance->splitRealImag(m_pfRealInput, m_pfImagInput, m_pfFreqInput);


        int nBlockIr = 0;
        while ((static_cast<float>(nBlockIr) < (static_cast<float>(m_iLengthOfIr) / static_cast<float>(m_iblockSize))) || (nBlockIr == 0))
        {
            for (int i = m_iblockSize * nBlockIr; i < m_iblockSize * (nBlockIr + 2); i++)
            {
                if (nBlockIr == 0) m_pfTimeIr[i] = (i < m_iblockSize || (i - m_iblockSize) > m_iLengthOfIr - 1) ? 0 : m_pfImpulseResponse[i - m_iblockSize] * fftBlockSize;
                else m_pfTimeIr[i - m_iblockSize * nBlockIr] = (i < m_iblockSize* (nBlockIr + 1) && i < m_iLengthOfIr) ? m_pfImpulseResponse[i] * fftBlockSize : 0;
            }

            m_pCFftInstance->doFft(m_pfFreqIr, m_pfTimeIr);
            m_pCFftInstance->splitRealImag(m_pfRealIr, m_pfImagIr, m_pfFreqIr);

            for (int i = 0; i < (fftBlockSize); i++)
            {
                m_pfRealConv[i] = (m_pfRealInput[i] * m_pfRealIr[i]) - (m_pfImagInput[i] * m_pfImagIr[i]);
                m_pfImagConv[i] = (m_pfRealInput[i] * m_pfImagIr[i]) + (m_pfImagInput[i] * m_pfRealIr[i]);
            }
            m_pCFftInstance->mergeRealImag(m_pfFreqConv, m_pfRealConv, m_pfImagConv);
            m_pCFftInstance->doInvFft(m_pfTmpConv, m_pfFreqConv);

            if (m_bLongBlock)
            {
                for (int i = 0; i < iLengthOfBuffers; i++)
                {
                    pfOutputBuffer[i] += (m_pfTmpConv[i] * (1.0f / (fftBlockSize)));
                }
                }
            else
            {
                for (int i = m_iblockSize * (nBlockIr + nBlockInput); i < m_iblockSize * (nBlockIr + nBlockInput + 2); i++)
                {
                    pfOutputBuffer[i] += (m_pfTmpConv[i] * (1.0f / (fftBlockSize)));
                }
            }
            nBlockIr++;
        }
        nBlockInput++;
    }
    delete[] m_pfFreqInput;
    m_pfFreqInput = 0;
    delete[] m_pfFreqIr;
    m_pfFreqIr = 0;
    delete[] m_pfFreqConv;
    m_pfFreqConv = 0;


    delete[] m_pfTimeInput;
    m_pfTimeInput = 0;
    delete[] m_pfTimeIr;
    m_pfTimeIr = 0;

    delete[] m_pfRealInput;
    m_pfRealInput = 0;
    delete[] m_pfImagInput;
    m_pfImagInput = 0;
    delete[] m_pfRealIr;
    m_pfRealIr = 0;
    delete[] m_pfImagIr;
    m_pfImagIr = 0;
    delete[] m_pfRealConv;
    m_pfRealConv = 0;
    delete[] m_pfImagConv;
    m_pfImagConv = 0;
    delete[] m_pfTmpConv;
    m_pfTmpConv = 0;

    return Error_t::kNoError;
}

