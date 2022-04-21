
#include "FastConv.h"

CFastConv::CFastConv( void )
{
}

CFastConv::~CFastConv( void )
{
    delete[] m_pfImpulseResponse;
    delete[] m_pfTail;
    delete[] m_pfInputTail;
    delete[] m_pfBlockBuffer;

    delete[] m_pfTimeInput;
    delete[] m_pfTimeIr;
    delete[] m_pfFreqInput;
    delete[] m_pfFreqIr;
    delete[] m_pfFreqConv;

    delete[] m_pfRealInput;
    delete[] m_pfImagInput;
    delete[] m_pfRealIr;
    delete[] m_pfImagIr;
    delete[] m_pfRealConv;
    delete[] m_pfImagConv;
    delete[] m_pfTmpConv;

    m_pCFftInstance->resetInstance();
    CFft::destroyInstance(m_pCFftInstance);
    
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/)
{
    m_ConvType = eCompMode;
    m_pfImpulseResponse = new float [iLengthOfIr];
    m_pfTail = new float [iLengthOfIr-1];
    m_pfInputTail = new float[(2 * iLengthOfIr) - 1];
    m_pfBlockBuffer = new float[iBlockLength];

    m_pfTimeInput = new float[2 * iBlockLength];
    m_pfTimeIr = new float[2 * iBlockLength];

    m_pfFreqInput = new CFft::complex_t[2 * iBlockLength];
    m_pfFreqIr = new CFft::complex_t[2 * iBlockLength];
    m_pfFreqConv = new CFft::complex_t[2 * iBlockLength];

    m_pfRealInput = new float[2 * iBlockLength];
    m_pfImagInput = new float[2 * iBlockLength];
    m_pfRealIr = new float[2 * iBlockLength];
    m_pfImagIr = new float[2 * iBlockLength];
    m_pfRealConv = new float[2 * iBlockLength];
    m_pfImagConv = new float[2 * iBlockLength];
    m_pfTmpConv = new float[2 * iBlockLength];


    m_iLengthOfIr = iLengthOfIr;
    m_iblockSize = iBlockLength;

    CFft::createInstance(m_pCFftInstance);

    if (m_ConvType == CFastConv::ConvCompMode_t::kFreqDomain)
    {
        m_pCFftInstance->initInstance(iBlockLength, 1, CFft::kWindowHann, CFft::kNoWindow);
    }

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

        //for (int i = 0; i < iLengthOfBuffers; i++)
        //{
        //    pfOutputBuffer[i] = 0;
        //}
        //int nBlockInput = 0;
        //int nBlockIr = 0;
        //while (nBlockInput < (iLengthOfBuffers / m_iblockSize))
        //{
        //    while (nBlockIr < (m_iLengthOfIr / m_iblockSize))
        //    {
        //        //for (int i = 0; i < iLengthOfBuffers; i++) {
        //        //    pfOutputBuffer[i] = 0;
        //        //    for (int j = 0; j < m_iLengthOfIr; j++) {
        //        //        if (i - j < 0) break;
        //        //        pfOutputBuffer[i] += pfInputBuffer[i - j] * m_pfImpulseResponse[j];
        //        //    }
        //        //}
        //        for (int i = m_iblockSize * nBlockIr; i < m_iblockSize * (nBlockIr + 1); i++)
        //        {
        //            pfOutputBuffer[i] = +m_pfTmpConv[i];
        //        }
        //        nBlockIr++;
        //    }
        //    nBlockInput++;
        //}
        //return Error_t::kNoError;

    }
    else if (m_ConvType == CFastConv::ConvCompMode_t::kFreqDomain)
    {
        for (int i = 0; i < iLengthOfBuffers; i++) 
        {
            pfOutputBuffer[i] = 0;
        }

        int nBlockInput = 0;
        int nBlockIr = 0;
        while ((nBlockInput < (iLengthOfBuffers / m_iblockSize))||(nBlockInput==0&& (iLengthOfBuffers / m_iblockSize)==0))
        {
            for (int i = m_iblockSize * nBlockInput; i < m_iblockSize * (nBlockInput + 2); i++)
            {
                m_pfTimeInput[i] = (i < m_iblockSize * (nBlockInput + 1)) ? pfInputBuffer[i] : 0;
            }
            while ((nBlockIr < (m_iLengthOfIr / m_iblockSize)) || (nBlockIr == 0 && (iLengthOfBuffers / m_iblockSize) == 0))
            {
                for (int i = m_iblockSize * nBlockIr; i < m_iblockSize * (nBlockIr + 2); i++)
                {
                    m_pfTimeIr[i] = (i < m_iblockSize * (nBlockIr + 1)) ? m_pfImpulseResponse[i] : 0;
                }
                m_pCFftInstance->doFft(m_pfFreqInput, m_pfTimeInput);
                m_pCFftInstance->doFft(m_pfFreqIr, m_pfTimeIr);

                m_pCFftInstance->splitRealImag(m_pfRealInput, m_pfImagInput, m_pfFreqInput);
                m_pCFftInstance->splitRealImag(m_pfRealIr, m_pfImagIr, m_pfFreqIr);

                for (int i = 0; i < (2 * m_iblockSize); i++)
                {
                    m_pfRealConv[i] = m_pfRealInput[i] * m_pfRealIr[i];
                    m_pfImagConv[i] = m_pfImagInput[i] * m_pfImagIr[i];
                }
                m_pCFftInstance->mergeRealImag(m_pfFreqConv, m_pfRealConv, m_pfImagConv);

                m_pCFftInstance->doInvFft(m_pfTmpConv, m_pfFreqConv);

                for (int i = m_iblockSize * nBlockIr; i < m_iblockSize * (nBlockIr + 2); i++)
                {
                    pfOutputBuffer[i] =+ m_pfTmpConv[i];
                }
                nBlockIr++;
            }
            nBlockInput++;
        }
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
