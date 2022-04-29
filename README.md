Compilation and Execution Instructions: \
\
1.) Extract the folder from the tarball into a single location. Ensure that all the files extracted are in the same folder. \
There should be  4 *.cpp programs files (svmFormat.cpp, rad.cpp, hjpd.cpp, and hod.cpp), 1 folder called "dataset", the generated skeleton based representation data, and then a README file. \
All of these should be in the same directory. \
2.) To compile the programs open a terminal in the unzipped files directory and type "make". This will generate all the necessary executables. \
\
NOTE: The following three programs require an input argument of either "train" or "test". Using "train" will use the training dataset, "test" will use the testing dataset. \
\
RAD: \
To run in terminal after compiling: ./RAD (input arg: "train" or "test") \
HJPD: \
To run in terminal after compiling: ./HJPD (input arg: "train or "test")\
HOD: \
To run in terminal after compiling: ./HOD (input arg: "train" or "test") \
SVMF: \
To run in terminal after compiling: ./SVMF (input args: [data file to be formatted] [name of output file]) \
useSVM: \
To run in terminal after compiling: ./useSVM (input args: [C value] [gamma value] [input training data file name] [input testing data file name])
\
Implementation Information: \
For detected NAN values, the whole frame containing even a single NAN value is deleted. \
RAD: \
The Joints used for the angle and distance calculations are the joints 1, 4, 8, 12, 16, and 20. \
HJPD: \
The number of bins in each histogram is determined by taking the square root of the total number of data points being considered.\
The bin width is a determined from calculating the residual of the maximum data value and the minimum data value in the set, and then dividing by the number of bins. \
The histogram is normalized according to the total number of data points \
\
HOD: \
The number of bins used in the HOD histograms is set to a static 8 bins. \
The bin width is 45 degrees per bin. \
The histogram uses the distance's magnitude as the real-value count and then is normalized by the total distance traveled in the joint's trajectory.

SVMF: \
Function to format the data created in deliverable one. Follows the required libSVM format of:
<label> <index1>:<value1> <index2>:<value2>...


