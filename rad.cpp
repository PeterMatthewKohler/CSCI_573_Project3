#include <glob.h>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath> 

using namespace std;

typedef vector< vector<float> > Matrix;
typedef vector<float> Row;

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
    while (!indata.eof()) { // Keep reading until end of file
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

float euclDist(float x1, float y1, float z1, float x2, float y2, float z2) {
    float x = abs(x1 - x2);
    float y = abs(y1 - y2);
    float z = abs(z1 - z2);

    float dist = sqrt( pow(x,2) + pow(y,2) + pow(z,2) );
    return dist;
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


// Just print all data in 5 per row format for all joints in all frames (for error checking)
void dispData(vector< vector<float> >& input) {
    int rows = 0;
    for (auto i = 0; i < input.size(); i++){
        for (auto j = 0; j < 5; j++) {
            cout << input[i][j] << " ";
            if (j == 4) {
                cout << endl;
                rows++;}
            }
        }
    cout << "Rows = " << rows << endl;    
}   




int main() {

    // Multi dimensional Array
    // Row =[Frame_#    Joint_#     X       Y       Z]]
    // Ex:  [1          1           0.055   -0.467  2.699]
    vector< vector<float> > data_array;
    vector< vector<float> > dist_array;
    int frames = 0; // Num Frames per text file
    // Get filenames of all textfiles in directory
    vector<string> result = glob("*.txt"); 
    for(size_t i = 0; i< result.size(); ++i){
        cout << result[i] << '\n';
        data_array = getData(result[i]);
        frames = data_array.size()/20;
        dist_array = calcDistances(data_array);
        dispData(dist_array);
    }

    



}