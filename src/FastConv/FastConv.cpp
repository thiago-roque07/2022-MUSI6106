
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
    if (iLengthOfIr < 1) {
        return Error_t::kFunctionInvalidArgsError;
    }

    m_ConvType = eCompMode;
    m_pfImpulseResponse = new float [iLengthOfIr];
    m_pfTail = new float [iLengthOfIr-1];
    m_pfInputTail = new float[(2 * iLengthOfIr) - 1];
    m_pfBlockBuffer = new float[iBlockLength];

    m_iLengthOfIr = iLengthOfIr;
    m_iblockSize = iBlockLength;

    if (m_ConvType == CFastConv::ConvCompMode_t::kFreqDomain)
    {
        CFft::createInstance(m_pCFftInstance);
        m_pCFftInstance->initInstance(2 * m_iblockSize, 1, CFft::kWindowHann, CFft::CFft::kPreWindow);
    }

    for (int i = 0; i < iLengthOfIr; i++)
    {
        m_pfImpulseResponse[i] = pfImpulseResponse[i];
    }

    return Error_t::kNoError;
}

Error_t CFastConv::reset()
{
    delete[] m_pfImpulseResponse;
    delete[] m_pfTail;
    delete[] m_pfInputTail;
    delete[] m_pfBlockBuffer;

    m_pCFftInstance->resetInstance();
    CFft::destroyInstance(m_pCFftInstance);

    return Error_t::kNoError;
}

Error_t CFastConv::process (float* pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers )
{
    if (m_iblockSize > iLengthOfBuffers) m_bLongBlock = true;

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
    
    if (m_ConvType == CFastConv::ConvCompMode_t::kTimeDomain)
    {
        for (int i = 0; i < iLengthOfBuffers; i++) {
            pfOutputBuffer[i] = 0;
            for (int j = 0; j < m_iLengthOfIr; j++) {
                if (i - j < 0) break;
                pfOutputBuffer[i] += pfInputBuffer[i - j] * m_pfImpulseResponse[j];
            }
        }
        return Error_t::kNoError;

    }
    else if (m_ConvType == CFastConv::ConvCompMode_t::kFreqDomain)
    {
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
        while ((nBlockInput < (iLengthOfBuffers / m_iblockSize)) || (nBlockInput == 0 && (iLengthOfBuffers / m_iblockSize) == 0))
        {
            for (int i = m_iblockSize * nBlockInput; i < m_iblockSize * (nBlockInput + 2); i++)
            {
                if(nBlockInput == 0) m_pfTimeInput[i] = (i < m_iblockSize || (i - m_iblockSize) > iLengthOfBuffers - 1) ? 0 : pfInputBuffer[i - m_iblockSize];
                else m_pfTimeInput[i] = (i < m_iblockSize * (nBlockInput + 1) && i < iLengthOfBuffers) ? pfInputBuffer[i] : 0;
            }
            int nBlockIr = 0;
            while ((nBlockIr < (m_iLengthOfIr / m_iblockSize)) || (nBlockIr == 0 && (iLengthOfBuffers / m_iblockSize) == 0))
            {
                for (int i = m_iblockSize * nBlockIr; i < m_iblockSize * (nBlockIr + 2); i++)
                {
                    if (nBlockIr == 0) m_pfTimeIr[i] = (i < m_iblockSize || (i - m_iblockSize) > m_iLengthOfIr - 1) ? 0 : m_pfImpulseResponse[i - m_iblockSize];
                    else m_pfTimeIr[i] = (i < m_iblockSize * (nBlockIr + 1) && i < m_iLengthOfIr) ? m_pfImpulseResponse[i] : 0;
                }
                m_pCFftInstance->doFft(m_pfFreqInput, m_pfTimeInput);
                m_pCFftInstance->doFft(m_pfFreqIr, m_pfTimeIr);

                for (int i = 0; i < fftBlockSize; i++)
                {
                    m_pfFreqInput[i] *= fftBlockSize;
                    m_pfFreqIr[i] *= fftBlockSize;
                }
                m_pCFftInstance->splitRealImag(m_pfRealInput, m_pfImagInput, m_pfFreqInput);
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
                        pfOutputBuffer[i] += (m_pfTmpConv[i] * (1.0f/ (fftBlockSize)));
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
        delete[] m_pfFreqIr;
        delete[] m_pfFreqConv;

        delete[] m_pfTimeInput;
        delete[] m_pfTimeIr;

        delete[] m_pfRealInput;
        delete[] m_pfImagInput;
        delete[] m_pfRealIr;
        delete[] m_pfImagIr;
        delete[] m_pfRealConv;
        delete[] m_pfImagConv;
        delete[] m_pfTmpConv;

        return Error_t::kNoError;
    }
    else return Error_t::kFunctionInvalidArgsError;
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer)
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
