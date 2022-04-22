#include <glob.h>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath> 
#include <math.h>
#include <algorithm>
#include <unistd.h>
#include <memory>

using namespace std;

typedef vector< vector<float> > Matrix;
typedef vector<float> Row;

struct jointCoord {  // Struct Holding Coordinates of Inidividual Joints
    float x, y, z;  // X, Y, and Z coordinates of Joint
    public:
        jointCoord(float x_in, float y_in, float z_in) {x = x_in; y = y_in; z = z_in;}   // Constructor
};

// Prototypes
vector<string> glob(const string& pattern);
Matrix getData(string filename);
Matrix checkNAN(Matrix data);
Matrix jointPosDiff(Matrix& input, string coord);
Row computeHistogram(Matrix& input, int frames);
string getcwd();

// Main Function
int main(int argc, char** argv) {
    // Determine Working Directory Path
    string workingDirectory = getcwd();
    string outFileName;
    ofstream output_file;
    // Based on argument inputs either work on Test or Train datasets
    if (argc > 1) {
        if (!strcmp(argv[1], "train")) {
            workingDirectory.append("/dataset/train");
            std::cout << "Using training dataset." << endl;
            // Output File
            output_file.open("hjpd_d1");
            outFileName = "hjpd_d1";

            }
        else if (!strcmp(argv[1], "test")) {
            workingDirectory.append("/dataset/test");
            std::cout << "Using testing dataset." << endl;
            // Output File
            output_file.open("hjpd_d1.t");
            outFileName = "hjpd_d1.t";
        }
        else {
            //workingDirectory.append("/dataset/test");
            std::cout << "No input argument given. Defaulting to training data." << endl;
            // Output File
            output_file.open("hjpd_d1");
            outFileName = "hjpd_d1";
        }
    }

    // Set new working directory
    chdir(workingDirectory.c_str()); 

    // Arrays to Store data and histograms
    Matrix temp_data_array;
    Matrix final_data_array;
    Matrix tempDist;
    
    Row featureVector;
    Row tempHist;

    int frames; // Num Frames per text file
    // Get filenames of all textfiles in directory
    vector<string> result = glob("*skeleton_proj.txt"); 
    for(size_t i = 0; i< result.size(); ++i){
        std::cout << "Opening file: " << result[i] << '\n';
        temp_data_array = getData(result[i]);               // Get data values from individual text file
        final_data_array = checkNAN(temp_data_array);        // Check for NaN values in joint data, and delete individual joint if found.
       
        frames = final_data_array.size()/20;                 // Calc total number of frames
        vector<string> dim = {"X", "Y", "Z"};
        for (int i = 0; i < dim.size(); i++) {
            // Generate Distance Values for X, Y, or Z coordinate
            tempDist = jointPosDiff(final_data_array, dim[i]);
            // Generate Histogram of X, Y, or Z values for entire data instance
            tempHist = computeHistogram(tempDist, frames);
            int sum = 0;
            for (int i = 0; i < tempHist.size(); i++){
                sum += tempHist[i];
            }
            // Append Histogram to feature vector
            featureVector.insert(featureVector.end(), tempHist.begin(), tempHist.end());
            // Clear vectors
            tempHist.clear();
            tempDist.clear();

        }

        // When Finished Populating Feature Vector, Write to File text.txt in directory.
        for (const auto &value : featureVector) {
            output_file << value << " ";
        }
        output_file << "\n";    // New line for next feature vector
        // Clear feature vector
        featureVector.clear();
    }

    // Close file
    output_file.close();
    std::cout << "All done. Writing feature vectors to file: " << outFileName << endl;

}

vector<string> glob(const string& pattern) {
    glob_t glob_result = {0}; // zero initialize

    // do the glob operation
    int return_value = ::glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);

    if(return_value != 0) throw runtime_error(strerror(errno));

    // collect all the filenames into a std::vector<std::string>
    // using the vector constructor that takes two iterators
    vector<string> filenames(
        glob_result.gl_pathv, glob_result.gl_pathv + glob_result.gl_pathc);

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}

Matrix getData(string filename) {
    // Define Matrix to hold data for this text file
    Matrix data;

    // Pull data 
    ifstream indata;
    float temp;
    string strTemp;

    indata.open(filename); // opens the file
    if(!indata) { // file couldn't be opened
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }

    indata >> strTemp;
    temp = strtof(strTemp.c_str(), NULL);

    while (!indata.eof())
    { // Keep reading until end of file
        // Populate Rows
        // Define row to hold data for that row
        Row row(5);
        auto index = 0;
        while (index < 5)
        {
            row[index] = temp;
            indata >> strTemp;
            temp = strtof(strTemp.c_str(), NULL);
            index++;
        }
        data.push_back(row);
    }
    
    return data;
}

// Looks for NaN values in data array. If finds, deletes the row.
Matrix checkNAN(Matrix data) {
    int count = 0;
    int size = data.size();
    while(count < size){
        if ( (data[count].size() > 0) && (isnan(data[count][2]) || isnan(data[count][3]) || isnan(data[count][4])) ) {      // Delete whole frame
            count = count - (count%20); // Go back to beginning of frame 
            int i=0;
            while (i < 20){     // Delete all joints from that frame
                data.erase(data.begin() + count);
                size = data.size();
                i++;
            }

        }
        else
            ++count;
    }
    Matrix tempHolder = data;
    return tempHolder;
}

// Calculate distances from joints
Matrix jointPosDiff(Matrix& input, string coord) {
    Matrix distances;
    int index;
    if (coord == "X"){index = 2;}
    if (coord == "Y"){index = 3;}
    if (coord == "Z"){index = 4;}
    
    for (int i = 0; i < input.size(); i = i + 20) { // i = Frame Number
        Row row(19);    // Distance from Joints 2 to 20 with respect to Joint 1
        // JPD Calcs for each joint with reference to skeleton centroid (joint 1)
        for (int j = 1; j < 20; j++){               // j = Joint Number
            row[j-1] = input[i+j][index] - input[i][index]; 
            
        }
        distances.push_back(row);
    }

    return distances;
}

// Histogram of X, Y, or Z
Row computeHistogram(Matrix& input, int frames) {
    // Determine number of bins
    int numData = input.size();   // 5 data points per row
    int bins = sqrt(numData);       // Number of bins determined by square root of number of data points
    float minVal, maxVal;


    // Find Maximum and Minimum values in the input data array
    vector<float>::iterator it_min, it_max;
    for (int i = 0; i < input.size(); i++) {
        it_min = min_element(input[i].begin(), input[i].end());
        it_max = max_element(input[i].begin(), input[i].end());
        
        if (*it_min < minVal) {minVal = *it_min;}
        if (*it_max > maxVal) {maxVal = *it_max;}
    }
    // Populate the Histogram
    Row histogram (bins, 0);
    float binWidth = (maxVal - minVal) / bins;
    for (int i = 0; i < input.size(); i++) {
        for (int j = 0; j < input[i].size(); j++) {
            for (int k = 0; k < bins; k++) {
                if (input[i][j] > (minVal + binWidth*k) && (input[i][j] <= (minVal + binWidth*(k+1))) ) {histogram[k] = histogram[k] + 1;}
            }
        }

    }
    float sum = 0;
    // Normalize Histogram by total frame count
    for (int i = 0; i < histogram.size(); i++) {
        histogram[i] /= frames;
        sum += histogram[i];
    }

    return histogram;
    
}

// Function to cleanly get working directory
string getcwd()
 {
     string out;
     char* buff = new char[255];    // Allocate memory
     out = string(getcwd(buff,255));
     delete[] buff;                 // Always remember to delete! :)
     return out;
 }


