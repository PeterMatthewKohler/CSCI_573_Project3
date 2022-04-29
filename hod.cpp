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

typedef vector<vector<vector<float>>> Array;
typedef vector<vector<float>> Matrix;
typedef vector<float> Row;

#define PI 3.14159; // For Angle Calculation

struct jointCoord {  // Struct Holding Coordinates of Inidividual Joints
    float x, y, z;  // X, Y, and Z coordinates of Joint
    public:
        jointCoord(float x_in, float y_in, float z_in) {x = x_in; y = y_in; z = z_in;}   // Constructor
};

// Prototypes
vector<string> glob(const string& pattern);
vector<Array> splitHalf(Array& input, int frames);
vector<Array> splitFourths(Array& input, int frames);
Matrix getData(string filename);
Matrix checkNAN(Matrix data);
Matrix getCoord(Matrix& input, int frames, string dim);
Array getTraj(Matrix& coord1, Matrix& coord2);
Array getAngDist(Array& input);
float euclDist(float x1, float y1, float x2, float y2);
float calcAngle(float x1, float y1, float x2, float y2);
Matrix computeHistogram(Array& input);
string getcwd();

// ------- Main Function --------- //
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
            output_file.open("hod_d1");
            outFileName = "hod_d1";

            }
        else if (!strcmp(argv[1], "test")) {
            workingDirectory.append("/dataset/test");
            std::cout << "Using testing dataset." << endl;
            // Output File
            output_file.open("hod_d1.t");
            outFileName = "hod_d1.t";
        }
        else {
            //workingDirectory.append("/dataset/test");
            std::cout << "No input argument given. Defaulting to training data." << endl;
            // Output File
            output_file.open("hod_d1");
            outFileName = "hod_d1";
        }
    }

    // Set new working directory
    chdir(workingDirectory.c_str()); 

    // Arrays to Store data and histograms
    Row featureVector;
    Matrix temp_data_array;
    Matrix final_data_array;
    Matrix coordsX, coordsY, coordsZ;
    Array xyAngDist, yzAngDist, xzAngDist;
    vector<Array> xyHalfAngDist, yzHalfAngDist, xzHalfAngDist;
    vector<Array> xyFourthAngDist, yzFourthAngDist, xzFourthAngDist;
    Matrix xyHOD, yzHOD, xzHOD;
    Array xyHalfHOD, yzHalfHOD, xzHalfHOD;
    Array xyFourthHOD, yzFourthHOD, xzFourthHOD;
    // 3D Trajectory Matrices
    Array xyTraj, yzTraj, xzTraj;
    vector<Array> xyTrajHalf, yzTrajHalf, xzTrajHalf;   // 2 Arrays, Index 0 -> 1st half, Index 1 -> 2nd half
    vector<Array> xyTrajQuarters, yzTrajQuarters, xzTrajQuarters;   // 4 Arrays, Index 0-3 -> Quarters 1-4 

    int frames; // Num Frames per text file
    // Get filenames of all textfiles in directory
    vector<string> result = glob("*skeleton_proj.txt"); 
    for(size_t i = 0; i< result.size(); ++i){
        std::cout << "Opening file: " << result[i] << '\n';
        temp_data_array = getData(result[i]);               // Get data values from individual text file
        final_data_array = checkNAN(temp_data_array);        // Check for NaN values in joint data, and delete individual joint if found.
        
        frames = final_data_array.size()/20;                 // Calc total number of frames
        // X, Y, Z coords of all joints over all frames
        coordsX = getCoord(final_data_array, frames, "X");
        coordsY = getCoord(final_data_array, frames, "Y");
        coordsZ = getCoord(final_data_array, frames, "Z");
        
        // Generate 2D trajectories
        xyTraj = getTraj(coordsX, coordsY);
        yzTraj = getTraj(coordsY, coordsZ);
        xzTraj = getTraj(coordsX, coordsZ);

        // Split Trajectories into halves
        xyTrajHalf = splitHalf(xyTraj, frames);
        yzTrajHalf = splitHalf(yzTraj, frames);
        xzTrajHalf = splitHalf(xzTraj, frames);

        // Split Trajectories into Fourths
        xyTrajQuarters = splitFourths(xyTraj, frames);
        yzTrajQuarters = splitFourths(yzTraj, frames);
        xzTrajQuarters = splitFourths(yzTraj, frames);

        // Generate Angles and Distance Values for each joint along set of points in trajectory
        // Full Trajectory Angles and Distances
        xyAngDist = getAngDist(xyTraj);
        yzAngDist = getAngDist(yzTraj);
        xzAngDist = getAngDist(xzTraj);

        // Half Trajectory Angles and Distances
        // Indexing: [Portion of Traj][Joint#][0(Ang)/1(Dist)][Frame]
        for (int i = 0; i < 2; i++){
            Array tempXY, tempYZ, tempXZ;
            tempXY = getAngDist(xyTrajHalf[i]);
            tempYZ = getAngDist(yzTrajHalf[i]);
            tempXZ = getAngDist(xzTrajHalf[i]);
            xyHalfAngDist.push_back(tempXY);
            yzHalfAngDist.push_back(tempYZ);
            xzHalfAngDist.push_back(tempXZ);
        }

        // Fourths Trajectory Angles and Distances
        // Indexing: [Portion of Traj][Joint#][0(Ang)/1(Dist)][Frame]
        for (int i = 0; i < 4; i++){
            Array tempXY, tempYZ, tempXZ;
            tempXY = getAngDist(xyTrajQuarters[i]);
            tempYZ = getAngDist(yzTrajQuarters[i]);
            tempXZ = getAngDist(xzTrajQuarters[i]);
            xyFourthAngDist.push_back(tempXY);
            yzFourthAngDist.push_back(tempYZ);
            xzFourthAngDist.push_back(tempXZ);
        }

        // Compute HODs for all 20 joints using full trajectory (2D Matrix: [Row = Histogram for Joint][Histogram Bin Value])
        xyHOD = computeHistogram(xyAngDist);
        yzHOD = computeHistogram(yzAngDist);
        xzHOD = computeHistogram(xzAngDist);

        // Compute HODs for Half Trajectories                   (3D Matrix: [Which half of Traj][Histogram for Joint][Histogram Bin Value])         
        for (int i = 0; i < 2; i++){
            Matrix tempXY, tempYZ, tempXZ;
            tempXY = computeHistogram(xyHalfAngDist[i]);
            tempYZ = computeHistogram(yzHalfAngDist[i]);
            tempXZ = computeHistogram(xzHalfAngDist[i]);
            xyHalfHOD.push_back(tempXY);
            yzHalfHOD.push_back(tempYZ);
            xzHalfHOD.push_back(tempXZ);
        }

        // Compute HODs for Quarter Trajectories                (3D Matrix: [Which Quarter of Traj][Histogram for Joint][Histogram Bin Value])
        for (int i = 0; i < 4; i++){
            Matrix tempXY, tempYZ, tempXZ;
            tempXY = computeHistogram(xyFourthAngDist[i]);
            tempYZ = computeHistogram(yzFourthAngDist[i]);
            tempXZ = computeHistogram(xzFourthAngDist[i]);
            xyFourthHOD.push_back(tempXY);
            yzFourthHOD.push_back(tempYZ);
            xzFourthHOD.push_back(tempXZ);
        }

        // Create Temporal Pyramid For All 20 Joints in XY Plane
        for (int i = 0; i < 20; i++){
            Row tempPyramid;
            // Insert HOD for full trajectory at first level
            tempPyramid.insert(tempPyramid.end(), xyHOD[i].begin(), xyHOD[i].end());    
            // Insert 2 HODs for Half Trajectories at 2nd level
            for (int j = 0; j < 2; j++){
                tempPyramid.insert(tempPyramid.end(), xyHalfHOD[j][i].begin(), xyHalfHOD[j][i].end());
            }
            // Insert 4 HODs for Quarter Trajectories at 3rd Level
            for (int j = 0; j < 4; j++) {
                tempPyramid.insert(tempPyramid.end(), xyFourthHOD[j][i].begin(), xyFourthHOD[j][i].end());
            }
            //Insert Temporal Pyramid Into Feature Vector
            featureVector.insert(featureVector.end(), tempPyramid.begin(), tempPyramid.end());       
        }
        // Create Temporal Pyramid For All 20 Joints in YZ Plane
        for (int i = 0; i < 20; i++){
            Row tempPyramid;
            // Insert HOD for full trajectory at first level
            tempPyramid.insert(tempPyramid.end(), yzHOD[i].begin(), yzHOD[i].end());    
            // Insert 2 HODs for Half Trajectories at 2nd level
            for (int j = 0; j < 2; j++){
                tempPyramid.insert(tempPyramid.end(), yzHalfHOD[j][i].begin(), yzHalfHOD[j][i].end());
            }
            // Insert 4 HODs for Quarter Trajectories at 3rd Level
            for (int j = 0; j < 4; j++) {
                tempPyramid.insert(tempPyramid.end(), yzFourthHOD[j][i].begin(), yzFourthHOD[j][i].end());
            }
            //Insert Temporal Pyramid Into Feature Vector
            featureVector.insert(featureVector.end(), tempPyramid.begin(), tempPyramid.end());       
        }
                // Create Temporal Pyramid For All 20 Joints in XZ Plane
        for (int i = 0; i < 20; i++){
            Row tempPyramid;
            // Insert HOD for full trajectory at first level
            tempPyramid.insert(tempPyramid.end(), xzHOD[i].begin(), xzHOD[i].end());    
            // Insert 2 HODs for Half Trajectories at 2nd level
            for (int j = 0; j < 2; j++){
                tempPyramid.insert(tempPyramid.end(), xzHalfHOD[j][i].begin(), xzHalfHOD[j][i].end());
            }
            // Insert 4 HODs for Quarter Trajectories at 3rd Level
            for (int j = 0; j < 4; j++) {
                tempPyramid.insert(tempPyramid.end(), xzFourthHOD[j][i].begin(), xzFourthHOD[j][i].end());
            }
            //Insert Temporal Pyramid Into Feature Vector
            featureVector.insert(featureVector.end(), tempPyramid.begin(), tempPyramid.end());       
        }
        // When Finished Populating Feature Vector, Write to File text.txt in directory.
        for (const auto &value : featureVector) {
            output_file << value << " ";
        }
        output_file << "\n";    // New line for next feature vector

        // Clear Storage for Next Data Instance
        xyHalfAngDist.clear();
        yzHalfAngDist.clear();
        xzHalfAngDist.clear();
        xyFourthAngDist.clear();
        yzFourthAngDist.clear();
        xzFourthAngDist.clear();
        xyHalfHOD.clear();
        yzHalfHOD.clear();
        xzHalfHOD.clear();
        xyFourthHOD.clear();
        yzFourthHOD.clear();
        xzFourthHOD.clear();
        featureVector.clear();
    }
    
    // Close file
    output_file.close();
    std::cout << "All done. Writing feature vectors to file: " << outFileName << endl;
}

// Create glob of filenames for data import
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

// Returns vector with 2 Arrays(3D) that contain trajectories split into halves
// Indexing Example: [0-1 (Half of Traj)][Joint #][0(X)-1(Y)][frame #]
vector<Array> splitHalf(Array& input, int frames){
    vector<Array> splitOut;     // Return variable
    Array tempArr1, tempArr2;   // Temp variables for storing halves
        for (int i = 0; i < 20; i++){
            Matrix tempMat1, tempMat2;
            Row tempRowX1(frames/2), tempRowX2(frames/2), tempRowY1(frames/2), tempRowY2(frames/2);
            for (int j = 0; j < frames/2; j++) {
                tempRowX1[j] = input[i][0][j];
                tempRowX2[j] = input[i][0][j+(frames/2)];
                tempRowY1[j] = input[i][1][j];
                tempRowY2[j] = input[i][1][j+(frames/2)];

            }
            // First half of Trajectories
            tempMat1.push_back(tempRowX1);
            tempMat1.push_back(tempRowY1);
            // Second half
            tempMat2.push_back(tempRowX2);
            tempMat2.push_back(tempRowY2); 
            // Push Back Halves into Respective Arrays
            tempArr1.push_back(tempMat1);
            tempArr2.push_back(tempMat2);
            tempMat1.clear();
            tempMat2.clear();
        }
        splitOut.push_back(tempArr1);
        splitOut.push_back(tempArr2);

        return splitOut;
}

// Returns vector with 4 arrays(3D) that contain trajectories split into fourths
// Indexing Example: [0-3 (Quarter of Traj)][Joint #][0(X)-1(Y)][frame #]
vector<Array> splitFourths(Array& input, int frames){
    vector<Array> splitOut;     // Return variable
    Array tempArr1, tempArr2, tempArr3, tempArr4;   // Temp variables for storing halves
        for (int i = 0; i < 20; i++){
            Matrix tempMat1, tempMat2, tempMat3, tempMat4;
            Row tempRowX1(frames/4), tempRowX2(frames/4), tempRowY1(frames/4), tempRowY2(frames/4);
            Row tempRowX3(frames/4), tempRowX4(frames/4), tempRowY3(frames/4), tempRowY4(frames/4);
            for (int j = 0; j < frames/4; j++) {
                tempRowX1[j] = input[i][0][j];
                tempRowX2[j] = input[i][0][j+(frames/4)];
                tempRowX3[j] = input[i][0][j+(2*frames)/4];
                tempRowX4[j] = input[i][0][j+(3*frames)/4];
                tempRowY1[j] = input[i][1][j];
                tempRowY2[j] = input[i][1][j+(frames/4)];
                tempRowY3[j] = input[i][1][j+(2*frames)/4];
                tempRowY4[j] = input[i][1][j+(3*frames)/4];

            }
            // First Quarter of Trajectories
            tempMat1.push_back(tempRowX1);
            tempMat1.push_back(tempRowY1);
            // Second Quarter
            tempMat2.push_back(tempRowX2);
            tempMat2.push_back(tempRowY2); 
            // Third Quarter
            tempMat3.push_back(tempRowX3);
            tempMat3.push_back(tempRowY3);
            // Fourth Quarter
            tempMat4.push_back(tempRowX4);
            tempMat4.push_back(tempRowY4);
            // Push Back Quarters into Respective Arrays
            tempArr1.push_back(tempMat1);
            tempArr2.push_back(tempMat2);
            tempArr3.push_back(tempMat3);
            tempArr4.push_back(tempMat4);
            tempMat1.clear();
            tempMat2.clear();
            tempMat3.clear();
            tempMat4.clear();
        }
        // Push back Arrays after population
        splitOut.push_back(tempArr1);
        splitOut.push_back(tempArr2);
        splitOut.push_back(tempArr3);
        splitOut.push_back(tempArr4);

        return splitOut;
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

// Make Vector of Specific Coordinate(X,Y,Z) for For All Input Joints Over All Frames
// Row # is for Joint Number (Remember 0 based index)
// Column # is for individual point in trajectory (Frame Number)
Matrix getCoord(Matrix& input, int frames, string dim) {
    int index;
    if (dim == "X"){index = 2;}         // X
    else if (dim == "Y") {index = 3;}   // Y
    else {index = 4;}                   // Z
    Matrix pointTraj;   // Trajectory composed of all points that joint is at over all frames
    Row row(frames);    // Row to hold trajectory of each joint over all frames
    for (int i = 0; i < 20; i++){
        for (int j = 0; j < frames; j++){
            row[j] = input[(i) + (j*20)][index];
        }
        pointTraj.push_back(row);
    }
    return pointTraj;
}
// Makes 3D Array of trajectory for 2 dimensions for all joints in data instance
// Indexing Example: xyTraj[Joint#][0(X)/1(Y)][frame#]
Array getTraj(Matrix& coord1, Matrix& coord2) {
    Array traj;
    Matrix joint;

    for (int i = 0; i < 20; i++) {      // Loop through all 20 joints
        joint.push_back(coord1[i]);     // Get joint coords of first dimension in first row
        joint.push_back(coord2[i]);     // Get joint coords of second dimension in next row
        traj.push_back(joint);          // Insert 2D matrix into trajectory array
        joint.clear();                   // Clear joint array
    }
    return traj;
}

// Makes a 3D Array of Angle and Distance Values between Two Points in Each Joints Respective Trajectory
// Indexing Example: xyAngDist[Joint#][0(Angle)/1(Dist)][frame]
Array getAngDist(Array& input){
    int frames = input[0][0].size();
    Array outArr;           // 3D array of angle and distance values for all 20 joints
    Matrix joint;           // 2D Matrix for each joint holding all computed angle values (first row) and all computed distance values (second row)
    Row angle(frames);      // 1D row containing all calculated angle values
    Row distance(frames);   // 1D row containing all calculated distance magnitude values
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < frames-1; j++) {
            angle[j] = calcAngle(input[i][0][j], input[i][1][j], input[i][0][j+1], input[i][1][j+1]);
            distance[j] = euclDist(input[i][0][j], input[i][1][j], input[i][0][j+1], input[i][1][j+1]);
        }
        joint.push_back(angle);
        joint.push_back(distance);
        outArr.push_back(joint);
        joint.clear();
    }
    return outArr;
}

// Function for Euclidean Distance between two points
float euclDist(float x1, float y1, float x2, float y2) {
    float x = abs(x1 - x2);
    float y = abs(y1 - y2);


    float dist = sqrt( pow(x,2) + pow(y,2) );
    return dist;
}
// Function for calculating angle between two points with respect to third axis
float calcAngle(float x1, float y1, float x2, float y2){
    float theta = atan2((y2-y1),(x2-x1)) * 180/PI;

    if (theta < 0) {theta = theta + 360;}
    return abs(theta);
}

// Returns 2D array of 20 histograms. 
Matrix computeHistogram(Array& input) {
    Matrix outMat;  // Output matrix
    // Bin number is 8;
    int bins = 25;
    int binWidth = floor(360/bins);  // Degrees
    int frames = input[0][0].size();

    // Populate the Histogram
    for (int i = 0; i < input.size(); i++) {    // Loop through each joint
        Row histogram (bins, 0);                // Initialize to zero
        float totalDist = 0;                    // For normalization after histogram population
        for (int j = 0; j < frames; j++) {      // Loop through each angle value in matrix
            totalDist += input[i][1][j];
            for (int k = 0; k < bins; k++) {    // Loop through all the bins
                if (input[i][0][j] >= (0 + binWidth*k) && (input[i][0][j] < (0 + binWidth*(k+1))) ) {
                    histogram[k] = histogram[k] + input[i][1][j];   // Add distance value to total in histogram bin
                    } 
            }
        }
        // Normalize Histogram
        for (int a = 0; a < histogram.size(); a++){
            histogram[a] /= totalDist;
        }
        outMat.push_back(histogram);
    }
    return outMat;
}

// Function to cleanly get working directory
string getcwd() {
     string out;
     char* buff = new char[255];    // Allocate memory
     out = string(getcwd(buff,255));
     delete[] buff;                 // Always remember to delete! :)
     return out;
 }


