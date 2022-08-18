#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__
// basic file utilities


#include <string>

namespace seeg {

    bool IsFileExists(const std::string filename);
    bool CreateDirectory(const std::string directoryName);
}



#endif /* FILEUTILS_H_ */
