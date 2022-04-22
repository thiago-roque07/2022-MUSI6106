

#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Synthesis.h"

#include "Vector.h"
#include "FastConv.h"

#include "gtest/gtest.h"


namespace fastconv_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    class FastConv: public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_pCFastConv = new CFastConv();

            m_IrLength = 51;
            m_ImpulseLength = 10;
            m_Ir = new float[m_IrLength];
            m_Impulse = new float[m_ImpulseLength];

            for (int i = 0; i < m_IrLength; i++)
            {
                if (i < m_ImpulseLength) { m_Impulse[i] = 0; };
                //m_Ir[i] = std::rand();
                m_Ir[i] = 10;
            }
            // m_Impulse[3] = 1;
            m_Impulse[1] = 1;

        }

        virtual void TearDown()
        {
            delete[] m_Ir;
            delete[] m_Impulse;
        }

        CFastConv *m_pCFastConv = 0;
        float *m_Ir = 0;
        float *m_Impulse = 0;

        int m_IrLength = 0;
        int m_ImpulseLength = 0;
    };

    TEST_F(FastConv, Identity_TD)
    {
        float *pfOutput = new float[m_ImpulseLength];
        CVector::setZero(pfOutput, m_ImpulseLength);

        m_pCFastConv->init(m_Ir,m_IrLength,m_IrLength,CFastConv::kTimeDomain);
        m_pCFastConv->process(pfOutput, m_Impulse, m_ImpulseLength);

        CHECK_ARRAY_CLOSE(m_Ir, pfOutput+3, 7, 1e-3);

        delete[] pfOutput;
    }

    TEST_F(FastConv, FlushBuffer_TD)
    {
        float *pfOutput = new float[m_ImpulseLength];
        CVector::setZero(pfOutput, m_ImpulseLength);
        float* pfTail = new float[m_IrLength-1];
        CVector::setZero(pfTail, m_IrLength-1);

        m_pCFastConv->init(m_Ir,m_IrLength,m_IrLength,CFastConv::kTimeDomain);
        m_pCFastConv->process(pfOutput, m_Impulse, m_ImpulseLength);
        m_pCFastConv->flushBuffer(pfTail);

        CHECK_ARRAY_CLOSE(m_Ir+7, pfTail,44,1e-3);

        delete[] pfOutput;
        delete[] pfTail;
    }

    TEST_F(FastConv, Blocksize_TD)
    {
        float *pfInput = new float[10000];
        CVector::setZero(pfInput, 10000);
        pfInput[3] = 1.F;

        float *pfOutput = new float[10000];
        CVector::setZero(pfOutput, 10000);

        float* pfTail = new float[m_IrLength - 1];
        CVector::setZero(pfTail, m_IrLength - 1);

        int blockSizes[8] = { 1, 13, 1023, 2048, 1, 17, 5000, 1897};

        m_pCFastConv->init(m_Ir,m_IrLength,m_IrLength,CFastConv::kTimeDomain);
        for (int i = 0, j = 0; i < 8; j += blockSizes[i++])
            m_pCFastConv->process(pfOutput+j, pfInput+j, blockSizes[i]);

        for (int i = 0; i < m_IrLength && i + 3 < 10000; i++)
            EXPECT_NEAR(m_Ir[i], pfOutput[i+3], 1e-3);

        m_pCFastConv->flushBuffer(pfTail);

        for (int i = m_IrLength + 3; i < 10000; i++)
            EXPECT_EQ(pfOutput[i], 0);

        //for (int i = 0; i < 8; i++)
        //{
        //    m_pCFastConv->process(pfOutput, pfInput, blockSizes[i]);
        //}

        delete[] pfInput;
        delete[] pfOutput;
        delete[] pfTail;
    }

    TEST_F(FastConv, Identity_FD)
    {
        float *pfOutput = new float[m_ImpulseLength];
        CVector::setZero(pfOutput, 10);

        m_pCFastConv->init(m_Ir,m_IrLength,64,CFastConv::kFreqDomain);
        m_pCFastConv->process(pfOutput, m_Impulse, 10);

        CHECK_ARRAY_CLOSE(m_Ir, pfOutput+3, 7, 1e-3);

        delete[] pfOutput;
    }

    TEST_F(FastConv, FlushBuffer_FD)
    {
        float *pfOutput = new float[m_IrLength];
        CVector::setZero(pfOutput, m_IrLength);

        m_pCFastConv->init(m_Ir,m_IrLength,m_IrLength,CFastConv::kFreqDomain);
        m_pCFastConv->process(pfOutput, m_Impulse, 10);
        m_pCFastConv->flushBuffer(pfOutput);

        CHECK_ARRAY_CLOSE(m_Ir+7,pfOutput,44,1e-3);

        delete[] pfOutput;
    }

    TEST_F(FastConv, Blocksize_FD)
    {
        float *pfInput = new float[10000];
        CVector::setZero(pfInput, 10000);
        pfInput[3] = 1.F;

        float *pfOutput = new float[10000];
        CVector::setZero(pfOutput, 10000);

        float* pfTail = new float[m_IrLength - 1];
        CVector::setZero(pfTail, m_IrLength - 1);

        int blockSizes[8] = { 1, 13, 1023, 2048, 1, 17, 5000, 1897};

        m_pCFastConv->init(m_Ir,m_IrLength,256,CFastConv::kFreqDomain);
        for (int i = 0, j = 0; i < 8; j += blockSizes[i++])
            m_pCFastConv->process(pfOutput+j, pfInput+j, blockSizes[i]);

        for (int i = 0; i < m_IrLength && i + 3 < 10000; i++)
            EXPECT_NEAR(m_Ir[i], pfOutput[i+3], 1e-3);

        m_pCFastConv->flushBuffer(pfTail);

        for (int i = m_IrLength + 3; i < 10000; i++)
            EXPECT_EQ(pfTail[i], 0);

        delete[] pfInput;
        delete[] pfOutput;
        delete[] pfTail;
    }
}

#endif //WITH_TESTS