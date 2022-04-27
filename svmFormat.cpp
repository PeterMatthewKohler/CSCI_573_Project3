#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <sstream>

// Prototypes
void label(std::ifstream& inF, std::ofstream& outF, std::vector<int> labels);

// Main Function
int main(int argc, char** argv) {
    // Read in input file based on the first input argument
    std::ifstream input_file;  
    input_file.open(argv[1]);

    // Open a file for output based on the second input argument
    std::ofstream output_file;
    output_file.open(argv[2]);

    // Check Input Filename to determine if using Testing or Training Labels
    int strLength = strlen(argv[1]);
    char t = 't';
    bool test;
    if (argv[1][strLength-1] == t){test = 1;}   // If input file ends in "t" it is a test file
    else {test = 0;}                                    // and should use the test dataset labels

    // Vectors Containing Labels for Testing and Training
    std::vector<int> testLabels{8,  8,  8,  8,  8,  8,  8,  8,                      // 48 Total
                                10, 10, 10, 10, 10, 10, 10, 10,
                                12, 12, 12, 12, 12, 12, 12, 12,
                                13, 13, 13, 13, 13, 13, 13, 13,
                                15, 15, 15, 15, 15, 15, 15, 15,
                                16, 16, 16, 16, 16, 16, 16, 16};
    std::vector<int> trainLabels{8, 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,      // 72 Total
                                10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
                                12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                                13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
                                15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
                                16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

    // Apply the labels and other formatting required by libSVM
    if (test)   {label(input_file, output_file, testLabels);}
    else        {label(input_file, output_file, trainLabels);}

    // Close input and output files
    input_file.close();
    output_file.close();
}

void label(std::ifstream& inF, std::ofstream& outF, std::vector<int> labels){
    // Indices to increment 
    int valueIndex = 1;
    int fileIndex = 0;
    
    std::string line;

    while (std::getline(inF, line))    // read line-by-line
    {
        outF << labels[fileIndex];
        outF << " ";
        std::istringstream iss(line);  // then process each line separately
        float value;
        while (iss >> value)        // Iterate through all values in feature vector
        {
            outF << valueIndex << ":" << value << " ";
            valueIndex++;
        }
        outF << "\n";           // New line after all values have been read
        fileIndex++;            // Increment File Index to get next label
        valueIndex = 1;         // Reset valueIndex back to 1;        
    }
}

