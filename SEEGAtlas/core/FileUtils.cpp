#include "FileUtils.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <QDir>

using namespace std;

namespace seeg {

    bool IsFileExists(const string filename) {
        bool exists = false;
        ifstream ifile(filename.c_str(), ifstream::in);
        exists = ifile ? true : false;
        ifile.close();
        return exists;
    }

    bool CreateDirectory(const std::string directoryName) {
        int rc = QDir().mkdir(directoryName.c_str());
        return true;
    }



}
