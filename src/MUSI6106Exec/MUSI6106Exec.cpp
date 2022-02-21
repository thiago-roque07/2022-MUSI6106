
#include <iostream>
#include <ctime>
#include <math.h> 

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

#define PI          3.141592

// local function declarations
void    showClInfo ();
int     test1();
int     test2();
int     test3(int filtType, int blockSize_1, int blockSize_2);
int     test4(int filtType);
int     test5(int filtType);


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
    
    //FiltType = CCombFilterIf::CombFilterType_t::kCombFIR;
    float GainValue = 0.0f;
    float DelayValueInSec = 0.1f;
    float MaxDelayInSec = 5.0f;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout << "No input information" << endl;
        cout << "Entering test mode" << endl;
        cout << "..." << endl;
        test = true;
    }
    else if (argc == 5)
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        sOutputFileDelayPath = sInputFilePath + "_delay.txt";
        sFilterType = argv[2];
        sFilterGain = argv[3];
        sFilterDelay = argv[4];

        GainValue = std::stof(sFilterGain);
        DelayValueInSec = std::stof(sFilterDelay);
    }
    else
    {
        cout << "Input information not recognized" << endl;
        cout << "Enter information as: [filePath FilterType FilterCoef FilterDelay]" << endl;
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


        if ((ppfAudioData == 0) || (ppfAudioDelayData == 0))
        {
            CAudioFileIf::destroy(phAudioFile);
            CCombFilterIf::destroy(pComb);
            hOutputFile.close();
            hOutputDelayFile.close();
            return -1;
        }

        time = clock();

        //////////////////////////////////////////////////////////////////////////////
        // Initialize Comb Filter

        pComb->init(FiltType, MaxDelayInSec, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
        pComb->setParam(gain, GainValue);
        pComb->setParam(delay, DelayValueInSec);

        //////////////////////////////////////////////////////////////////////////////
        // get audio data and write it to the output text file (one column per channel)
        while (!phAudioFile->isEof())
        {
            // set block length variable
            long long iNumFrames = kBlockSize;

            // read data (iNumOfFrames might be updated!)
            phAudioFile->readData(ppfAudioData, iNumFrames);

            // filter signal
            pComb->process(ppfAudioData, ppfAudioDelayData, iNumFrames);

            lastFrameSize = iNumFrames;
            numOfFrames++;

            cout << "\r" << "reading and writing";

            // write file
            for (int i = 0; i < iNumFrames; i++)
            {
                for (int c = 0; c < stFileSpec.iNumChannels; c++)
                {
                    hOutputFile << ppfAudioData[c][i] << "\t";
                    hOutputDelayFile << ppfAudioDelayData[c][i] << "\t";
                }
                hOutputFile << endl;
                hOutputDelayFile << endl;
            }
        }

        cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;


        //////////////////////////////////////////////////////////////////////////////
        // clean-up (close files and free memory)
        CAudioFileIf::destroy(phAudioFile);
        CCombFilterIf::destroy(pComb);

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
        int testResult = 0;

        testResult += test1();
        testResult += test2();
        testResult += test3(0, 512, 1024); // test 3 for FIR filter
        testResult += test3(1, 512, 1024); // test 3 for IIR filter
        testResult += test4(0);
        testResult += test4(1);
        testResult += test5();

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

    int PassTest = 0;

    int numChannel = 1;
    float sampleRate = 10000;
    float freq = 100;
    float Nsamples = 0.5 * sampleRate;
    float** inputSine = new float* [numChannel];
    float** output = new float* [numChannel];
    inputSine[0] = new float[Nsamples];
    output[0] = new float[Nsamples];

    CCombFilterIf::CombFilterType_t FiltType = CCombFilterIf::CombFilterType_t::kCombFIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;

    float GainValue = -1.0f;
    float DelayValueInSec = 0.01f;
    float MaxDelayInSec = 1.0f;
    float fSampleRateInHz = sampleRate;

    for (int i = 0; i < Nsamples; ++i) {
        inputSine[0][i] = static_cast<float>(sin(2 * PI * freq * i / sampleRate));
    }

    CCombFilterIf* pComb = 0;
    CCombFilterIf::create(pComb);

    pComb->init(FiltType, MaxDelayInSec, fSampleRateInHz, 1);
    pComb->setParam(gain, GainValue);
    pComb->setParam(delay, DelayValueInSec);
    pComb->process(inputSine, output, Nsamples);

    for (int i = 0; i < Nsamples; i++)
    {
        if (abs(output[0][i]) > 0.1)
        {
            PassTest++;
        }
    }
    cout << "test 1 result:" << PassTest << endl;

    delete[] inputSine[0];
    delete[] inputSine;
    delete[] output[0];
    delete[] output;
    CCombFilterIf::destroy(pComb);



    if (!PassTest)
    {
        cout << "test 1 OK" << endl << endl;
        return 0;
    }
    else
    {
        cout << "test 1 fail! Test result should be zero for approval!" << endl << endl;
        return 2;
    }
}

int test2()
{
    // IIR: amount of magnitude increase/decrease if input freq matches feedback

    int PassTest = 0;

    int numChannel = 1;
    float sampleRate = 10000;
    float freq = 100;
    float Nsamples = sampleRate;
    float** inputSine = new float* [numChannel];
    float** output = new float* [numChannel];
    inputSine[0] = new float[Nsamples];
    output[0] = new float[Nsamples];

    CCombFilterIf::CombFilterType_t FiltType = CCombFilterIf::CombFilterType_t::kCombIIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;

    float GainValue = 0.5f;
    float DelayValueInSec = 0.01f;
    float MaxDelayInSec = 1.0f;
    float fSampleRateInHz = sampleRate;

    for (int i = 0; i < Nsamples; ++i) {
        inputSine[0][i] = static_cast<float>(sin(2 * PI * freq * i / sampleRate));
    }

    CCombFilterIf* pComb = 0;
    CCombFilterIf::create(pComb);

    pComb->init(FiltType, MaxDelayInSec, fSampleRateInHz, 1);
    pComb->setParam(gain, GainValue);
    pComb->setParam(delay, DelayValueInSec);
    pComb->process(inputSine, output, Nsamples);

    for (int i = 0; i < 99; i++)
    {
        int sinePeak = i * 100 + 25;
        if (output[0][sinePeak] < 0.9)
        {
            cout << output[0][sinePeak];
            PassTest++;
        }
    }
    cout << "test 2 result:" << PassTest << endl;
   
    delete[] inputSine[0];
    delete[] inputSine;
    delete[] output[0];
    delete[] output;
    CCombFilterIf::destroy(pComb);

    if (!PassTest) 
    {
        cout << "test 2 OK" << endl << endl;
        return 0;
    }
    else
    {
        cout << "test 2 fail! Test result should be zero for approval!" << endl << endl;
        return 4;
    }
}

int test3(int filtType, int blockSize_1, int blockSize_2)
{
    // FIR/IIR: correct result for VARYING input block size

    int testCounter = 0;
    int numOfFrames = 0;
    long lastFrameSize = 0;

    std::string sInputFilePath = "C:/Users/thiag/Documents/Git-repos/2022-MUSI6106/audio/sweep.wav";
    std::string sOutputFilePath_1,
        sOutputFilePath_2;

    sOutputFilePath_1 = sInputFilePath + "_blockSize_1.txt";
    sOutputFilePath_2 = sInputFilePath + "_blockSize_2.txt";

    int kBlockSize_1 = blockSize_1;
    int kBlockSize_2 = blockSize_2;

    float** ppfAudioData_1 = 0;
    float** ppfAudioData_2 = 0;
    float** ppfAudioDelayData_1 = 0;
    float** ppfAudioDelayData_2 = 0;

    std::fstream hOutputFile_1;
    std::fstream hOutputFile_2;

    CAudioFileIf* phAudioFile = 0;
    CAudioFileIf* phAudioFile_read_1 = 0;
    CAudioFileIf* phAudioFile_read_2 = 0;

    CAudioFileIf::FileSpec_t stFileSpec;

    CCombFilterIf* pComb = 0;
    CCombFilterIf::create(pComb);

    CCombFilterIf::CombFilterType_t FiltTypeFIR = CCombFilterIf::CombFilterType_t::kCombFIR;
    CCombFilterIf::CombFilterType_t FiltTypeIIR = CCombFilterIf::CombFilterType_t::kCombIIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;

    float GainValue = 0.5f;
    float DelayValueInSec = 0.1f;
    float MaxDelayInSec = 1.0f;


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
    hOutputFile_1.open(sOutputFilePath_1.c_str(), std::ios::out);
    hOutputFile_2.open(sOutputFilePath_2.c_str(), std::ios::out);
    if (!hOutputFile_1.is_open() || !hOutputFile_2.is_open())
    {
        cout << "Text file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData_1 = new float* [stFileSpec.iNumChannels];
    ppfAudioData_2 = new float* [stFileSpec.iNumChannels];
    ppfAudioDelayData_1 = new float* [stFileSpec.iNumChannels];
    ppfAudioDelayData_2 = new float* [stFileSpec.iNumChannels];


    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfAudioData_1[i] = new float[kBlockSize_1];
        ppfAudioData_2[i] = new float[kBlockSize_2];

        ppfAudioDelayData_1[i] = new float[kBlockSize_1];
        ppfAudioDelayData_2[i] = new float[kBlockSize_2];
    }

    if ((ppfAudioData_1 == 0) || (ppfAudioData_2 == 0))
    {
        CAudioFileIf::destroy(phAudioFile);
        CCombFilterIf::destroy(pComb);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Initialize Comb Filter

    if (filtType == 0)
    {
        pComb->init(FiltTypeFIR, MaxDelayInSec, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    }
    else if (filtType == 1)
    {
        pComb->init(FiltTypeIIR, MaxDelayInSec, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    }
    pComb->setParam(gain, GainValue);
    pComb->setParam(delay, DelayValueInSec);


    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames_1 = kBlockSize_1;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData_1, iNumFrames_1);

        lastFrameSize = iNumFrames_1;
        numOfFrames++;

        // filter signal
        pComb->process(ppfAudioData_1, ppfAudioDelayData_1, iNumFrames_1);

        // write file for block size 1
        for (int i = 0; i < iNumFrames_1; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputFile_1 << ppfAudioDelayData_1[c][i] << "\t";
            }
            hOutputFile_1 << endl;
        }



        // set block length variable
        long long iNumFrames_2 = kBlockSize_2;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData_2, iNumFrames_2);

        // filter signal
        pComb->process(ppfAudioData_2, ppfAudioDelayData_2, iNumFrames_2);

        // write file for block size 2
        for (int i = 0; i < iNumFrames_2; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputFile_2 << ppfAudioDelayData_2[c][i] << "\t";
            }
            hOutputFile_2 << endl;
        }
    }


    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);
    CCombFilterIf::destroy(pComb);


    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioData_1[i];
        delete[] ppfAudioData_2[i];
        delete[] ppfAudioDelayData_1[i];
        delete[] ppfAudioDelayData_2[i];
    }

    delete[] ppfAudioData_1;
    delete[] ppfAudioData_2;
    delete[] ppfAudioDelayData_1;
    delete[] ppfAudioDelayData_2;

    ppfAudioData_1 = 0;
    ppfAudioData_2 = 0;
    ppfAudioDelayData_1 = 0;
    ppfAudioDelayData_2 = 0;


    //////////////////////////////////////////////////////////////////////////////
    // open the saved files for comparison
    CAudioFileIf::create(phAudioFile_read_1);
    phAudioFile_read_1->openFile(sOutputFilePath_1, CAudioFileIf::kFileRead);
    CAudioFileIf::create(phAudioFile_read_2);
    phAudioFile_read_2->openFile(sOutputFilePath_2, CAudioFileIf::kFileRead);

    if ((!phAudioFile_read_1->isOpen()) || (!phAudioFile_read_1->isOpen()))
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFile_read_1);
        CAudioFileIf::destroy(phAudioFile_read_2);
        return -1;
    }
    phAudioFile_read_1->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData_1 = new float* [stFileSpec.iNumChannels];
    ppfAudioData_2 = new float* [stFileSpec.iNumChannels];


    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfAudioData_1[i] = new float[kBlockSize_1];
        ppfAudioData_2[i] = new float[kBlockSize_2];
    }

    if ((ppfAudioData_1 == 0) || (ppfAudioData_2 == 0))
    {
        CAudioFileIf::destroy(phAudioFile_read_1);
        CAudioFileIf::destroy(phAudioFile_read_2);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (!phAudioFile_read_1->isEof())
    {
        // set block length variable
        long long iNumFrames = 512;

        // read data (iNumOfFrames might be updated!)
        phAudioFile_read_1->readData(ppfAudioData_1, iNumFrames);
        phAudioFile_read_2->readData(ppfAudioData_2, iNumFrames);

        lastFrameSize = iNumFrames;
        numOfFrames++;

        // compare content of both files
        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                testCounter += ppfAudioData_1[c][i] - ppfAudioData_2[c][i];
            }

        }
    }
    cout << "test 3 result: " << testCounter << endl;
    if (abs(testCounter) < 0.1)
    {
        cout << "test 3 OK" << endl;
        return 0;
    }
    else return 8;
}

int test4(int filtType)
{
    // FIR/IIR: correct processing for zero input signal

    int PassTest = 0;

    int numChannel = 1;
    int Nsamples = 500;
    float** input = new float* [numChannel];
    float** output = new float* [numChannel];
    input[0] = new float[Nsamples];
    output[0] = new float[Nsamples];

    CCombFilterIf::CombFilterType_t FiltTypeFIR = CCombFilterIf::CombFilterType_t::kCombFIR;
    CCombFilterIf::CombFilterType_t FiltTypeIIR = CCombFilterIf::CombFilterType_t::kCombIIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;

    float GainValue = 0.5f;
    float DelayValueInSec = 0.01f;
    float MaxDelayInSec = 1.0f;
    float fSampleRateInHz = 44100;

    for (int i = 0; i < Nsamples; ++i) {
        input[0][i] = 0;
    }

    CCombFilterIf* pComb = 0;
    CCombFilterIf::create(pComb);

    if (filtType == 0)
    {
        pComb->init(FiltTypeFIR, MaxDelayInSec, fSampleRateInHz, 1);
    }
    else if (filtType == 1)
    {
        pComb->init(FiltTypeIIR, MaxDelayInSec, fSampleRateInHz, 1);
    }

    pComb->setParam(gain, GainValue);
    pComb->setParam(delay, DelayValueInSec);
    pComb->process(input, output, Nsamples);

    for (int i = 0; i < Nsamples; i++)
    {
        if (abs(output[0][i]) > 0.05)
        {
            //cout << output[0][i] << endl;
            PassTest++;
        }
    }
    cout << "test 4 result:" << PassTest << endl;

    delete[] input[0];
    delete[] input;
    delete[] output[0];
    delete[] output;
    CCombFilterIf::destroy(pComb);

    if (!PassTest)
    {
        cout << "test 4 OK" << endl;
        return 0;
    }
    else
    {
        cout << "test 4 fail! Test result should be zero for approval!" << endl << endl;
        return 16;
    }
}

int test5()
{
    // One more additional MEANINGFUL test to verify your filter implementation
    // If gain is set to ZERO, the two files should be the same

    bool PassTest = false;

    if (PassTest) return 0;
    else return 32;
}


int test5(int filtType)
{

    int testCounter = 0;
    int numOfFrames = 0;
    long lastFrameSize = 0;

    std::string sInputFilePath = "C:/Users/thiag/Documents/Git-repos/2022-MUSI6106/audio/sweep.wav";
    std::string sOutputFilePath_1,
        sOutputFilePath_2;

    sOutputFilePath_1 = sInputFilePath + "orig.txt";
    sOutputFilePath_2 = sInputFilePath + "filtered.txt";

    int kBlockSize_1 = 1024;

    float** ppfAudioData_1 = 0;
    float** ppfAudioData_2 = 0;
    float** ppfAudioDelayData_1 = 0;
    float** ppfAudioDelayData_2 = 0;

    std::fstream hOutputFile_1;
    std::fstream hOutputFile_2;

    CAudioFileIf* phAudioFile = 0;
    CAudioFileIf* phAudioFile_read_1 = 0;
    CAudioFileIf* phAudioFile_read_2 = 0;

    CAudioFileIf::FileSpec_t stFileSpec;

    CCombFilterIf* pComb = 0;
    CCombFilterIf::create(pComb);

    CCombFilterIf::CombFilterType_t FiltTypeFIR = CCombFilterIf::CombFilterType_t::kCombFIR;
    CCombFilterIf::CombFilterType_t FiltTypeIIR = CCombFilterIf::CombFilterType_t::kCombIIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;

    float GainValue = 0f;
    float DelayValueInSec = 0.5f;
    float MaxDelayInSec = 1.0f;


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
    hOutputFile_1.open(sOutputFilePath_1.c_str(), std::ios::out);
    hOutputFile_2.open(sOutputFilePath_2.c_str(), std::ios::out);
    if (!hOutputFile_1.is_open() || !hOutputFile_2.is_open())
    {
        cout << "Text file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData_1 = new float* [stFileSpec.iNumChannels];
    ppfAudioData_2 = new float* [stFileSpec.iNumChannels];
    ppfAudioDelayData_1 = new float* [stFileSpec.iNumChannels];
    ppfAudioDelayData_2 = new float* [stFileSpec.iNumChannels];


    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfAudioData_1[i] = new float[kBlockSize_1];
        ppfAudioData_2[i] = new float[kBlockSize_2];

        ppfAudioDelayData_1[i] = new float[kBlockSize_1];
        ppfAudioDelayData_2[i] = new float[kBlockSize_2];
    }

    if ((ppfAudioData_1 == 0) || (ppfAudioData_2 == 0))
    {
        CAudioFileIf::destroy(phAudioFile);
        CCombFilterIf::destroy(pComb);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Initialize Comb Filter

    if (filtType == 0)
    {
        pComb->init(FiltTypeFIR, MaxDelayInSec, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    }
    else if (filtType == 1)
    {
        pComb->init(FiltTypeIIR, MaxDelayInSec, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    }
    pComb->setParam(gain, GainValue);
    pComb->setParam(delay, DelayValueInSec);


    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames_1 = kBlockSize_1;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData_1, iNumFrames_1);

        lastFrameSize = iNumFrames_1;
        numOfFrames++;

        // filter signal
        pComb->process(ppfAudioData_1, ppfAudioDelayData_1, iNumFrames_1);

        // write file for block size 1
        for (int i = 0; i < iNumFrames_1; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputFile_1 << ppfAudioDelayData_1[c][i] << "\t";
                hOutputFile_2 << ppfAudioDelayData_1[c][i] << "\t";
            }
            hOutputFile_1 << endl;
        }
    }


    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);
    CCombFilterIf::destroy(pComb);


    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioData_1[i];
        delete[] ppfAudioData_2[i];
        delete[] ppfAudioDelayData_1[i];
        delete[] ppfAudioDelayData_2[i];
    }

    delete[] ppfAudioData_1;
    delete[] ppfAudioData_2;
    delete[] ppfAudioDelayData_1;
    delete[] ppfAudioDelayData_2;

    ppfAudioData_1 = 0;
    ppfAudioData_2 = 0;
    ppfAudioDelayData_1 = 0;
    ppfAudioDelayData_2 = 0;


    //////////////////////////////////////////////////////////////////////////////
    // open the saved files for comparison
    CAudioFileIf::create(phAudioFile_read_1);
    phAudioFile_read_1->openFile(sOutputFilePath_1, CAudioFileIf::kFileRead);
    CAudioFileIf::create(phAudioFile_read_2);
    phAudioFile_read_2->openFile(sOutputFilePath_2, CAudioFileIf::kFileRead);

    if ((!phAudioFile_read_1->isOpen()) || (!phAudioFile_read_1->isOpen()))
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFile_read_1);
        CAudioFileIf::destroy(phAudioFile_read_2);
        return -1;
    }
    phAudioFile_read_1->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData_1 = new float* [stFileSpec.iNumChannels];
    ppfAudioData_2 = new float* [stFileSpec.iNumChannels];


    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfAudioData_1[i] = new float[kBlockSize_1];
        ppfAudioData_2[i] = new float[kBlockSize_2];
    }

    if ((ppfAudioData_1 == 0) || (ppfAudioData_2 == 0))
    {
        CAudioFileIf::destroy(phAudioFile_read_1);
        CAudioFileIf::destroy(phAudioFile_read_2);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (!phAudioFile_read_1->isEof())
    {
        // set block length variable
        long long iNumFrames = 512;

        // read data (iNumOfFrames might be updated!)
        phAudioFile_read_1->readData(ppfAudioData_1, iNumFrames);
        phAudioFile_read_2->readData(ppfAudioData_2, iNumFrames);

        lastFrameSize = iNumFrames;
        numOfFrames++;

        // compare content of both files
        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                testCounter += ppfAudioData_1[c][i] - ppfAudioData_2[c][i];
            }

        }
    }
    cout << "test 3 result: " << testCounter << endl;
    if (abs(testCounter) < 0.1)
    {
        cout << "test 3 OK" << endl;
        return 0;
    }
    else return 8;
}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

