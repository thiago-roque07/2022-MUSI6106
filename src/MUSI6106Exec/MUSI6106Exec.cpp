
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"
#include "FastConv.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{


    std::string             sInputFilePath,                 //!< file paths
        sOutputFilePath,
        sIrFilePath;

    static const int            kFileBlockSize = 1023,
        kProcBlockSize = 8192;
    long long                   iNumFrames = kFileBlockSize;
    long long                   iIrLength = 0;
    int                         iNumChannels;

    clock_t                     time = 0;

    float* pfInputAudio = 0;
    float* pfIrAudio = 0;
    float* pfOutputAudio = 0;

    CAudioFileIf* phAudioInFile = 0;
    CAudioFileIf* phIrFile = 0;
    CAudioFileIf* phAudioOutFile = 0;

    CAudioFileIf::FileSpec_t    stInFileSpec,
        stIrFileSpec;

    CFastConv* pCConv = 0;

    showClInfo();


    // command line args
    if (argc < 4)
    {
        cout << "Incorrect number of arguments!" << endl;
        return -1;
    }
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];
    sIrFilePath = argv[3];

    ///////////////////////////////////////////////////////////////////////////
    // open wave files
    CAudioFileIf::create(phAudioInFile);
    CAudioFileIf::create(phAudioOutFile);
    CAudioFileIf::create(phIrFile);

    phAudioInFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioInFile->getFileSpec(stInFileSpec);
    if (!phAudioInFile->isOpen() || stInFileSpec.iNumChannels != 1)
    {
        cout << "Input file open error!";

        CAudioFileIf::destroy(phAudioInFile);
        return -1;
    }

    phIrFile->openFile(sIrFilePath, CAudioFileIf::kFileRead);
    phIrFile->getFileSpec(stIrFileSpec);
    if (!phIrFile->isOpen() || stIrFileSpec.iNumChannels != 1 || stIrFileSpec.fSampleRateInHz != stInFileSpec.fSampleRateInHz)
    {
        cout << "Input file open error!";

        CAudioFileIf::destroy(phAudioInFile);
        CAudioFileIf::destroy(phIrFile);
        return -1;
    }
    else
    {
        int iCurrIdx = 0;
        phIrFile->getLength(iIrLength);
        pfIrAudio = new float[iIrLength];
        while (!phIrFile->isEof())
        {
            float* pfTmp = &pfIrAudio[iCurrIdx];
            phIrFile->readData(&pfTmp, iNumFrames);
            iCurrIdx += iNumFrames;
        }

    }
    phAudioOutFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stInFileSpec);
    iNumChannels = stInFileSpec.iNumChannels;
    if (!phAudioOutFile->isOpen())
    {
        cout << "Output file cannot be initialized!";

        CAudioFileIf::destroy(phAudioInFile);
        CAudioFileIf::destroy(phIrFile);
        CAudioFileIf::destroy(phAudioOutFile);
        return -1;
    }

    // creat instance
    pCConv = new CFastConv();

    // allocate memory
    pfInputAudio = new float[kFileBlockSize];
    pfOutputAudio = new float[kFileBlockSize];

    ////////////////////////////////////////////////////////////////////////////
    // freq domain
    pCConv->init(pfIrAudio, iIrLength, kProcBlockSize, CFastConv::kFreqDomain);

    time = clock();

    // processing
    iNumFrames = kFileBlockSize;
    while (!phAudioInFile->isEof())
    {
        phAudioInFile->readData(&pfInputAudio, iNumFrames);
        pCConv->process(pfOutputAudio, pfInputAudio, iNumFrames);
        phAudioOutFile->writeData(&pfOutputAudio, iNumFrames);
    }

    cout << "\nfreq domain processing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    ////////////////////////////////////////////////////////////////////////////
    // time domain
    pCConv->init(pfIrAudio, 10000, kProcBlockSize, CFastConv::kTimeDomain); // truncated here b/c it's so sloooooooow
    phAudioInFile->setPosition(static_cast<long long>(0));
    phAudioOutFile->setPosition(static_cast<long long>(0));

    time = clock();

    // processing
    iNumFrames = kFileBlockSize;
    while (!phAudioInFile->isEof())
    {
        phAudioInFile->readData(&pfInputAudio, iNumFrames);
        pCConv->process(pfOutputAudio, pfInputAudio, iNumFrames);
        phAudioOutFile->writeData(&pfOutputAudio, iNumFrames);
    }

    cout << "\ntime domain processing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;
    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioInFile);
    CAudioFileIf::destroy(phAudioOutFile);
    CAudioFileIf::destroy(phIrFile);

    delete[] pfInputAudio;
    delete[] pfIrAudio;
    delete[] pfOutputAudio;

    delete pCConv;

    // all done
    return 0;

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;

    return;
}

