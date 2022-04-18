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
            m_IrLength = 51;
            m_ImpulseLength = 10;
            m_Ir = new float[m_IrLength];
            m_Impulse = new float[m_ImpulseLength];

            for (int i = 0; i < m_IrLength; i++)
            {
                if (i < m_ImpulseLength) { m_Impulse[i] = 0; };
                m_Ir[i] = std::rand();
            }
            m_Impulse[3] = 1;

            // CFastConv::init();
            
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
    }

    TEST_F(FastConv, FlushBuffer_TD)
    {
    }

    TEST_F(FastConv, Blocksize_TD)
    {
    }

    TEST_F(FastConv, Identity_FD)
    {
    }

    TEST_F(FastConv, FlushBuffer_FD)
    {
    }

    TEST_F(FastConv, Blocksize_FD)
    {
    }
}

#endif //WITH_TESTS

