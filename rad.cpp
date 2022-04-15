#include <glob.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

std::vector<std::string> glob(const std::string& pattern) {
    glob_t glob_result = {0}; // zero initialize

    // do the glob operation
    int return_value = ::glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);

    if(return_value != 0) throw std::runtime_error(std::strerror(errno));

    // collect all the filenames into a std::vector<std::string>
    // using the vector constructor that takes two iterators
    std::vector<std::string> filenames(
        glob_result.gl_pathv, glob_result.gl_pathv + glob_result.gl_pathc);

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}

int main() {
    try { // catch exceptions
        vector<string> result = glob("*a0*"); // files with an "a" in the filename

        for(size_t i = 0; i< result.size(); ++i)
            cout << result[i] << '\n';

    } catch(const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}