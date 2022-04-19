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

// Function for Euclidean Distance
float euclDist(float x1, float y1, float z1, float x2, float y2, float z2) {
    float x = abs(x1 - x2);
    float y = abs(y1 - y2);
    float z = abs(z1 - z2);

    float dist = sqrt( pow(x,2) + pow(y,2) + pow(z,2) );
    return dist;
}

// Magnitude of Vector
float magnitude(vector<float> input){
    float sum = (input[0] * input[0]) + (input[1] * input[1]) + (input[2] * input[2]);
    return sqrt(sum);
}

// Dot Product Calculation
float dotProduct(vector<float> input1, vector<float> input2) {
    float dotp = (input1[0] * input2[0]) + (input1[1] * input2[1]) + (input1[2] * input2[2]);
    return dotp;
}

// Calculate distances from joints
Matrix jointPosDiff_X(Matrix& input) {
    Matrix distances;
    for (int i = 0; i < input.size(); i = i + 20) { // i = Frame Number
        Row row(19);    // Distance from Joints 2 to 20 with respect to Joint 1
        // JPD Calcs for each joint with reference to skeleton centroid (joint 1)
        for (int j = 1; j < 20; j++){               // j = Joint Number
            row[j-1] = input[i][j] - input[i][0]; 
        }
        distances.push_back(row);
    }


    return distances;
}


vector<float> computeHistogram(Matrix& input, int frames, int index) {
    // Determine number of bins
    int numData = input.size();   // 5 data points per row
    int bins = sqrt(numData);       // Number of bins determined by square root of number of data points
    float minVal, maxVal;
    minVal = maxVal = input[0][index];

    // Find Maximum and Minimum values in the input data array
    vector<float>::iterator it_min, it_max;
    for (int i = 0; i < input.size(); i++) {
        if (input[i][index] < minVal) {minVal = input[i][index];}
        if (input[i][index] > maxVal) {maxVal = input[i][index];}
    }
    // Populate the Histogram
    Row histogram (bins, 0);
    float binWidth = (maxVal - minVal) / bins;
    for (int i = 0; i < input.size(); i++) {
        for (int k = 0; k < bins; k++) {
            if (input[i][index] > (minVal + binWidth*k) && (input[i][index] <= (minVal + binWidth*(k+1))) ) {histogram[k] = histogram[k] + 1;}
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



int main(int argc, char** argv) {
    // Determine Working Directory Path
    string workingDirectory = getcwd();
    string outFileName;
    ofstream output_file;
    // Based on argument inputs either work on Test or Train datasets
    if (argc > 1) {
        std::cout << argv[1] << endl;
        std::cout << strcmp(argv[1], "train") << endl;
        if (!strcmp(argv[1], "train")) {
            workingDirectory.append("/dataset/train");
            std::cout << "Using training dataset." << endl;
            // Output File
            output_file.open("rad_d1");
            outFileName = "rad_d1";

            }
        else if (!strcmp(argv[1], "test")) {
            workingDirectory.append("/dataset/test");
            std::cout << "Using testing dataset." << endl;
            // Output File
            output_file.open("rad_d1.t");
            outFileName = "rad_d1.t";
        }
        else {
            //workingDirectory.append("/dataset/test");
            std::cout << "No input argument given. Defaulting to training data." << endl;
            // Output File
            output_file.open("rad_d1");
            outFileName = "rad_d1";
        }
    }

    // Set new working directory
    chdir(workingDirectory.c_str()); 

    // Arrays to Store data and histograms
    Matrix temp_data_array;
    Matrix temp_dist_array;
    Matrix temp_angle_array;
    Matrix final_data_array;
    
    Row featureVector;
    Row tempDistH;
    Row finalDistH;
    Row tempAngH;
    Row finalAngH;

    int frames; // Num Frames per text file
    // Get filenames of all textfiles in directory
    vector<string> result = glob("*skeleton_proj.txt"); 
    for(size_t i = 0; i< result.size(); ++i){
        std::cout << "Opening file: " << result[i] << '\n';
        temp_data_array = getData(result[i]);               // Get data values from individual text file
        final_data_array = checkNAN(temp_data_array);        // Check for NaN values in joint data, and delete individual joint if found.
        
        frames = final_data_array.size()/20;                 // Calc total number of frames
        temp_dist_array = calcDistances(final_data_array);   // Calculate distances
        temp_angle_array = calcAngles(final_data_array);     // Calculate angles
        // Generate Distance Histograms
        for (int i = 0; i < 5; i++) {
            tempDistH = computeHistogram(temp_dist_array, frames, i);  // Compute Histogram for Individual Distances
            finalDistH.insert(finalDistH.end(), tempDistH.begin(), tempDistH.end());    // Store Histogram
        }
        // Place Distance Histograms into 1D Vector
        featureVector.insert(featureVector.end(), finalDistH.begin(), finalDistH.end());
        // Generate Angle Histograms
        for (int i = 0; i < 5; i++) {
            tempAngH = computeHistogram(temp_angle_array, frames, i);  // Compute Histogram for Angles
            finalAngH.insert(finalAngH.end(), tempAngH.begin(), tempAngH.end());
        }
        // Append Angle Histograms into same 1D Vector
        featureVector.insert(featureVector.end(), finalAngH.begin(), finalAngH.end());

        // When Finished Populating Feature Vector, Write to File text.txt in directory.
        for (const auto &value : featureVector) {
            output_file << value << " ";
        }

        output_file << "\n";    // New line for next feature vector

        // Clear Histogram containers
        finalDistH.clear();
        finalAngH.clear();
        featureVector.clear();

    }

    // Close file
    output_file.close();
    std::cout << "All done. Writing feature vectors to file: " << outFileName << endl;

}







