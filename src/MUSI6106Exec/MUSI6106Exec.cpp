
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
    std::string sInputFilePath,                 //!< file paths
                sOutputFilePath;

    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfAudioData = 0;

    CAudioFileIf *phAudioFile = 0;
    std::fstream hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    CCombFilterIf   *pInstance = 0;
    CCombFilterIf::create(pInstance);
    
    showClInfo();

    pCRingBuff = new CRingBuffer<float>(kBlockSize);

    for (int i = 0; i < 5; i++)
    {
        pCRingBuff->putPostInc(1.F*i);
    }

    for (int i = 5; i < 30; i++)
    {
        pCRingBuff->getNumValuesInBuffer(); // should be five
        pCRingBuff->getPostInc(); // should be i-5
        pCRingBuff->putPostInc(1.F*i);
    }

    // all done
    return 0;

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}
