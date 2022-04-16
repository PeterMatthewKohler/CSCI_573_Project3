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

vector< vector<float> > getData(string filename) {
    // Define Matrix to hold data for this text file
    Matrix data;

    // Pull data 
    ifstream indata;
    float temp;

    indata.open(filename); // opens the file
    if(!indata) { // file couldn't be opened
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }
    indata >> temp;
    while (!indata.eof())
    { // Keep reading until end of file
        // Populate Rows
        // Define row to hold data for that row
        Row row(5);
        auto index = 0;
        while (index < 5)
        {
            row[index] = temp;
            indata >> temp;
            index++;
        }
        data.push_back(row);
    }
    
    return data;
}

vector< vector<float> > getData20(string filename) {
    // Define Matrix to hold data for this text file
    Matrix data;

    // Pull data 
    ifstream indata;
    float temp;

    indata.open(filename); // opens the file
    if(!indata) { // file couldn't be opened
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }
    indata >> temp;
    int i = 0;
    while (i < 20)
    { // Keep reading until end of file
        // Populate Rows
        // Define row to hold data for that row
        Row row(5);
        auto index = 0;
        while (index < 5)
        {
            row[index] = temp;
            indata >> temp;
            index++;
        }
        data.push_back(row);
        i++;
    }
    
    return data;
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

vector< vector<float> > calcDistances(vector< vector<float> >& input) {
    Matrix distances;
    for (int i = 0; i < input.size(); i = i + 20) {
        Row row(5);
        row[0] = euclDist(input[i][2], input[i][3], input[i][4], input[i+3][2], input[i+3][3], input[i+3][4] ); // Dist between Joints 1 and 4
        row[1] = euclDist(input[i][2], input[i][3], input[i][4], input[i+7][2], input[i+7][3], input[i+7][4] ); // Dist between Joints 1 and 8  
        row[2] = euclDist(input[i][2], input[i][3], input[i][4], input[i+11][2], input[i+11][3], input[i+11][4] ); // Dist between Joints 1 and 12
        row[3] = euclDist(input[i][2], input[i][3], input[i][4], input[i+15][2], input[i+15][3], input[i+15][4] ); // Dist between Joints 1 and 15
        row[4] = euclDist(input[i][2], input[i][3], input[i][4], input[i+19][2], input[i+19][3], input[i+19][4] ); // Dist between Joints 1 and 20
        distances.push_back(row);
    }


    return distances;
}

vector< vector<float> > calcAngles(vector< vector<float> >& input) {
    Matrix angles;

    for (int i = 0; i < input.size(); i = i + 20) {
        Row row(5);
        // Joint Coordinates
        jointCoord j1(input[i][2],       input[i][3],    input[i][4]);       // Joint 1
        jointCoord j4(input[i+3][2],     input[i+3][3],  input[i+3][4]);     // Joint 4
        jointCoord j8(input[i+7][2],     input[i+7][3],  input[i+7][4]);     // Joint 8
        jointCoord j12(input[i+11][2],   input[i+11][3], input[i+11][4]);    // Joint 12
        jointCoord j16(input[i+15][2],   input[i+15][3], input[i+15][4]);    // Joint 16
        jointCoord j20(input[i+19][2],   input[i+19][3], input[i+19][4]);    // Joint 20

        // Vectors
        vector<float> v1_4 = {(j4.x - j1.x), (j4.y - j1.y), (j4.z - j1.z)};     // From Joint 1 to 4
        vector<float> v1_8 = {(j8.x - j1.x), (j8.y - j1.y), (j8.z - j1.z)};     // From Joint 1 to 8
        vector<float> v1_12 = {(j12.x - j1.x), (j12.y - j1.y), (j12.z - j1.z)}; // From Joint 1 to 12
        vector<float> v1_16 = {(j16.x - j1.x), (j16.y - j1.y), (j16.z - j1.z)}; // From Joint 1 to 16
        vector<float> v1_20 = {(j20.x - j1.x), (j20.y - j1.y), (j20.z - j1.z)}; // From Joint 1 to 20

        // Calculate Angles between vectors
        row[0] = acos( dotProduct(v1_4, v1_8)/(magnitude(v1_4)*magnitude(v1_8)));
        row[1] = acos( dotProduct(v1_8, v1_16)/(magnitude(v1_8)*magnitude(v1_16)));
        row[2] = acos( dotProduct(v1_16, v1_20)/(magnitude(v1_16)*magnitude(v1_20)));
        row[3] = acos( dotProduct(v1_20, v1_12)/(magnitude(v1_20)*magnitude(v1_12)));
        row[4] = acos( dotProduct(v1_12, v1_4)/(magnitude(v1_12)*magnitude(v1_4)));
        angles.push_back(row);

    }
    return angles;
}

vector<float> computeHistogram(vector< vector<float> >& input, int frames, int index) {
    // Determine number of bins
    int numData = input.size();   // 5 data points per row
    int bins = sqrt(numData);       // Number of bins determined by square root of number of data points
    float minVal, maxVal;
    minVal = maxVal = input[0][0];

    // Find Maximum and Minimum values in the input data array
    vector<float>::iterator it_min, it_max;
    for (int i = 0; i < input.size(); i++) {
        it_min = min_element(input[i].begin(), input[i].end());
        it_max = max_element(input[i].begin(), input[i].end());
        if (*it_min < minVal) {minVal = *it_min;}
        if (*it_max > maxVal) {maxVal = *it_max;}
    }
    // Populate the Histogram
    vector<float> histogram (bins, 0);
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
            
    // cout << "Bins: " << bins << ", Binwidth: " << binWidth << ", Sum: " << sum << endl;
    // for (int i = 0; i < histogram.size(); i++){
    //     cout << histogram[i] << ", ";
    // }
    // cout << endl;

    return histogram;
    
}

// Just print all data in 5 per row format for all joints in all frames (for error checking)
void dispData(vector< vector<float> >& input) {
    int rows = 0;
    int nancount = 0;
    for (auto i = 0; i < input.size(); i++){
        for (auto j = 0; j < 5; j++) {
            if (isnan(input[i][j])){nancount++;}
            cout << input[i][j] << " ";
            if (j == 4) {
                cout << endl;
                rows++;}
            }
        }
    //cout << "nancount = " << nancount << endl;  
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
    // Based on argument inputs either work on Test or Train datasets
    if (argc > 1) {
        cout << argv[1] << endl;
        cout << strcmp(argv[1], "train") << endl;
        if (!strcmp(argv[1], "train")) {workingDirectory.append("/dataset/train");}
        else if (!strcmp(argv[1], "test")) {workingDirectory.append("/dataset/test");}
        else {workingDirectory.append("/dataset/test");}
    }

    // Set new working directory
    cout << workingDirectory << endl;
    chdir(workingDirectory.c_str()); 





    // Arrays to Store data and histograms
    vector< vector<float> > temp_data_array;
    vector< vector<float> > temp_dist_array;
    vector< vector<float> > temp_angle_array;
    vector<float> featureVector;
    vector<float> tempDistH;
    vector<float> finalDistH;
    vector<float> tempAngH;
    vector<float> finalAngH;

    int frames; // Num Frames per text file
    // Get filenames of all textfiles in directory
    vector<string> result = glob("*.txt"); 
    for(size_t i = 0; i< result.size(); ++i){
        cout << result[i] << '\n';
        temp_data_array = getData(result[i]);               // Get data values from individual text file
        frames = temp_data_array.size()/20;                 // Calc total number of frames
        temp_dist_array = calcDistances(temp_data_array);   // Calculate distances
        temp_angle_array = calcAngles(temp_data_array);     // Calculate angles
        // Generate Distance Histograms
        for (int i = 0; i < 5; i++) {
            tempDistH = computeHistogram(temp_dist_array, frames, i);  // Compute Histogram for Individual Distances
            finalDistH.insert(finalDistH.end(), tempDistH.begin(), tempDistH.end());    // Store Histogram
        }
        // Place Distance Histograms into 1D Feature Vector
        featureVector.insert(featureVector.end(), finalDistH.begin(), finalDistH.end());
        // Generate Angle Histograms
        for (int i = 0; i < 5; i++) {
            tempAngH = computeHistogram(temp_angle_array, frames, i);  // Compute Histogram for Angles
            finalAngH.insert(finalAngH.end(), tempAngH.begin(), tempAngH.end());
        }
        // Place Angle Histograms into 1D Feature Vector
        featureVector.insert(featureVector.end(), finalAngH.begin(), finalAngH.end());
        // Clear Histogram containers
        finalDistH.clear();
        finalAngH.clear();


    }

    // When Finished Populating Feature Vector, Write to File text.txt in directory.
    // ofstream output_file("test.txt");
    // for (const auto &e : featureVector) {output_file << e << " ";}



}