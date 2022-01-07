
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
 
    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
 
    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
 
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
 
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)

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

