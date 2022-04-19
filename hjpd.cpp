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
Matrix calcDistances(Matrix& input) {
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

Matrix calcAngles(Matrix& input) {
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
        Row v1_4 = {(j4.x - j1.x), (j4.y - j1.y), (j4.z - j1.z)};     // From Joint 1 to 4
        Row v1_8 = {(j8.x - j1.x), (j8.y - j1.y), (j8.z - j1.z)};     // From Joint 1 to 8
        Row v1_12 = {(j12.x - j1.x), (j12.y - j1.y), (j12.z - j1.z)}; // From Joint 1 to 12
        Row v1_16 = {(j16.x - j1.x), (j16.y - j1.y), (j16.z - j1.z)}; // From Joint 1 to 16
        Row v1_20 = {(j20.x - j1.x), (j20.y - j1.y), (j20.z - j1.z)}; // From Joint 1 to 20

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







