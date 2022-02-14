
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;


// local function declarations
void    showClInfo ();
int     test1();
int     test2();
int     test3();
int     test4();
int     test5();


/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    bool test = false;
    int numOfFrames = 0;
    long lastFrameSize = 0;

    std::string sInputFilePath,                 //!< file paths
        sOutputFilePath,
        sOutputFileDelayPath,
        sFilterType,
        sFilterGain,
        sFilterDelay;

    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfAudioData = 0;
    float **ppfAudioDelayData = 0;

    CAudioFileIf *phAudioFile = 0;
    std::fstream hOutputFile;
    std::fstream hOutputDelayFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    CCombFilterIf* pComb = 0;
    CCombFilterIf::create(pComb);


    CCombFilterIf::CombFilterType_t FiltType = CCombFilterIf::CombFilterType_t::kCombFIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;
    
    FiltType = CCombFilterIf::CombFilterType_t::kCombFIR;
    float GainValue = 0.0f;
    float DelayValueInSec = 0.0001f;
    float MaxDelayInSec = 5.0f;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        test = true;
    }
    else if (argc < 3)
    {
        //cout << "case 1" << endl;
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        sOutputFileDelayPath = sInputFilePath + "_delay.wav";
    }
    else
    {
        //cout << "case 2" << endl;
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        sOutputFileDelayPath = sInputFilePath + "_delay.wav";
        sFilterType = argv[2];
        sFilterGain = argv[3];
        sFilterDelay = argv[4];

        GainValue = std::stof(sFilterGain);
        DelayValueInSec = std::stof(sFilterDelay);
    }


    
    if (!test)
    {
        if (sFilterType == "IIR")
        {
            FiltType = CCombFilterIf::CombFilterType_t::kCombIIR;
        }
        else if (sFilterType == "FIR")
        {
            FiltType = CCombFilterIf::CombFilterType_t::kCombFIR;
        }
        else
        {
            cout << "Filter undefined!";
            return -1;
        }

        //////////////////////////////////////////////////////////////////////////////
        // open the input wave file
        CAudioFileIf::create(phAudioFile);
        phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
        if (!phAudioFile->isOpen())
        {
            cout << "Wave file open error!";
            CAudioFileIf::destroy(phAudioFile);
            return -1;
        }
        phAudioFile->getFileSpec(stFileSpec);

        //////////////////////////////////////////////////////////////////////////////
        // open the output text file
        hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
        if (!hOutputFile.is_open())
        {
            cout << "Text file open error!";
            CAudioFileIf::destroy(phAudioFile);
            return -1;
        }

        //////////////////////////////////////////////////////////////////////////////
        // open the output text file for the delayed version
        hOutputDelayFile.open(sOutputFileDelayPath.c_str(), std::ios::out);
        if (!hOutputDelayFile.is_open())
        {
            cout << "Text file open error!";
            CAudioFileIf::destroy(phAudioFile);
            return -1;
        }
        //////////////////////////////////////////////////////////////////////////////
        // allocate memory
        ppfAudioData = new float* [stFileSpec.iNumChannels];
        ppfAudioDelayData = new float* [stFileSpec.iNumChannels];
        for (int i = 0; i < stFileSpec.iNumChannels; i++) {
            ppfAudioData[i] = new float[kBlockSize];
            ppfAudioDelayData[i] = new float[kBlockSize];
        }


        if (ppfAudioData == 0)
        {
            CAudioFileIf::destroy(phAudioFile);
            hOutputFile.close();
            return -1;
        }
        if (ppfAudioData[0] == 0)
        {
            CAudioFileIf::destroy(phAudioFile);
            hOutputFile.close();
            return -1;
        }

        time = clock();


        //////////////////////////////////////////////////////////////////////////////
        // get audio data and write it to the output text file (one column per channel)
        while (!phAudioFile->isEof())
        {
            // set block length variable
            long long iNumFrames = kBlockSize;

            // read data (iNumOfFrames might be updated!)
            phAudioFile->readData(ppfAudioData, iNumFrames);
            lastFrameSize = iNumFrames;
            numOfFrames++;

            cout << "\r" << "reading and writing";

            // write
            for (int i = 0; i < iNumFrames; i++)
            {
                for (int c = 0; c < stFileSpec.iNumChannels; c++)
                {
                    hOutputFile << ppfAudioData[c][i] << "\t";
                }
                hOutputFile << endl;
            }
        }

        cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

        //////////////////////////////////////////////////////////////////////////////
        // Initialize Comb Filter
        long long combSize = ((numOfFrames - 1) * kBlockSize) + lastFrameSize;// - (DelayValueInSec / stFileSpec.fSampleRateInHz);
        pComb->init(FiltType, MaxDelayInSec, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
        pComb->setParam(gain, GainValue);
        pComb->setParam(delay, DelayValueInSec);
        pComb->process(ppfAudioData, ppfAudioDelayData, combSize);

        //////////////////////////////////////////////////////////////////////////////
        // get delayed audio data and write it to the output text file (one column per channel)

            // set block length variable
        long long iNumFrames = kBlockSize;


        // write
        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputDelayFile << ppfAudioDelayData[c][i] << "\t";
            }
            hOutputDelayFile << endl;
        }

        //////////////////////////////////////////////////////////////////////////////
        // clean-up (close files and free memory)
        CAudioFileIf::destroy(phAudioFile);
        hOutputFile.close();
        hOutputDelayFile.close();

        for (int i = 0; i < stFileSpec.iNumChannels; i++)
        {
            delete[] ppfAudioData[i];
            delete[] ppfAudioDelayData[i];
        }

        delete[] ppfAudioData;
        delete[] ppfAudioDelayData;
        ppfAudioData = 0;
        ppfAudioDelayData = 0;

        // all done
        return 0;
    }
    else
    {
        // Do test
        cout << "implement test here";
        int testResult = 0;

        testResult = +test1();
        testResult = +test2();
        testResult = +test3();
        testResult = +test4();
        testResult = +test5();

        if (!testResult)
        {
            cout << "No Errors";
            return 0;
        }
        else
        {
            cout << "Test Failed" << endl;
            cout << "Error code: " << testResult;
            return -1;
        }
    }
    

}

int test1()
{
    // FIR: Output is zero if input freq matches feedforward

    bool PassTest = false;

    CCombFilterIf::CombFilterType_t FiltType = CCombFilterIf::CombFilterType_t::kCombFIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;

    float GainValue = 0.0f;
    float DelayValueInSec = 0.0001f;
    float MaxDelayInSec = 5.0f;
    float fSampleRateInHz = 44100;
    int numChannel = 1;

    CCombFilterIf* pComb = 0;
    CCombFilterIf::create(pComb);

    pComb->init(FiltType, MaxDelayInSec, fSampleRateInHz, 1);
    pComb->setParam(gain, GainValue);
    pComb->setParam(delay, DelayValueInSec);
    // pComb->process(ppfAudioData, ppfAudioDelayData, numChannel);

    if (PassTest) return 0;
    else return 2;
}

int test2()
{
    // IIR: amount of magnitude increase/decrease if input freq matches feedback

    bool PassTest = false;

    CCombFilterIf::CombFilterType_t FiltType = CCombFilterIf::CombFilterType_t::kCombIIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;

    float GainValue = 0.0f;
    float DelayValueInSec = 0.0001f;
    float MaxDelayInSec = 5.0f;
    float fSampleRateInHz = 44100;
    int numChannel = 1;

    CCombFilterIf* pComb = 0;
    CCombFilterIf::create(pComb);

    pComb->init(FiltType, MaxDelayInSec, fSampleRateInHz, 1);
    pComb->setParam(gain, GainValue);
    pComb->setParam(delay, DelayValueInSec);
    // pComb->process(ppfAudioData, ppfAudioDelayData, numChannel);

    if (PassTest) return 0;
    else return 4;
}

int test3()
{
    // FIR/IIR: correct result for VARYING input block size

    bool PassTest = false;

    if (PassTest) return 0;
    else return 8;
}

int test4()
{
    // FIR/IIR: correct processing for zero input signal

    bool PassTest = false;

    if (PassTest) return 0;
    else return 16;
}

int test5()
{
    // One more additional MEANINGFUL test to verify your filter implementation

    bool PassTest = false;

    if (PassTest) return 0;
    else return 32;
}

void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

