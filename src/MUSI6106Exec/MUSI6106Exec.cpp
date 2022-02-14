
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;


// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    bool test = false;

    cout << "Init" << endl;
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
    cout << "Init2" << endl;
    CCombFilterIf::create(pComb);


    CCombFilterIf::CombFilterType_t FiltType = CCombFilterIf::CombFilterType_t::kCombFIR;
    CCombFilterIf::FilterParam_t gain = CCombFilterIf::FilterParam_t::kParamGain;
    CCombFilterIf::FilterParam_t delay = CCombFilterIf::FilterParam_t::kParamDelay;
    
    float GainValue = 0.0f;
    float DelayValueInSec = 0.001f;
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
        cout << "case 1" << endl;
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        sOutputFileDelayPath = sInputFilePath + "_delayed.txt";
    }
    else
    {
        cout << "case 2" << endl;
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        sOutputFileDelayPath = sInputFilePath + "_delayed.txt";
        sFilterType = argv[2];
        sFilterGain = argv[3];
        sFilterDelay = argv[4];
    }

    GainValue = std::stof(sFilterGain);
    DelayValueInSec = std::stof(sFilterDelay);
    
    if (!test)
    {
        if (sFilterType == "FIR")
        {
            FiltType = CCombFilterIf::CombFilterType_t::kCombFIR;
        }
        else if (sFilterType == "IIR")
        {
            FiltType = CCombFilterIf::CombFilterType_t::kCombIIR;
        }
        else
        {
            cout << "Filter type not recognized!";
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
        // open the output text file for teh delayed version
        hOutputDelayFile.open(sOutputFileDelayPath.c_str(), std::ios::out);
        if (!hOutputDelayFile.is_open())
        {
            cout << "Text 2 file open error!";
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
        // Initialize Comb Filter

        pComb->init(FiltType, MaxDelayInSec, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
        pComb->setParam(gain, GainValue);
        pComb->setParam(delay, DelayValueInSec);
        pComb->process(ppfAudioData, ppfAudioDelayData, stFileSpec.iNumChannels);

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
        // get audio data and write it to the output text file (one column per channel)
        while (!phAudioFile->isEof())
        {
            // set block length variable
            long long iNumFrames = kBlockSize;

            // read data (iNumOfFrames might be updated!)
            phAudioFile->readData(ppfAudioData, iNumFrames);

            cout << "\r" << "reading and writing";

            // write
            for (int i = 0; i < iNumFrames; i++)
            {
                for (int c = 0; c < stFileSpec.iNumChannels; c++)
                {
                    //hOutputFile << ppfAudioData[c][i] << "\t";
                    hOutputFile << ppfAudioDelayData[c][i] << "\t";
                }
                hOutputFile << endl;
            }
        }

        cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

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
        return 0;
    }
    

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

