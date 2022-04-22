
Compilation and Execution Instructions: \
1.) Extract the folder from the tarball into a single location. Ensure that all the files extracted are in the same folder. \
There should be five. 3 .cpp programs (rad.cpp, hjpd.cpp, and hod.cpp). There should be 1 folder called "dataset" and then a README. \
2.) To compile each of the programs use the following commands in a linux terminal: \
NOTE: Each of these programs requires an input argument of either "train" or "test". Selecting "train" will use the training dataset, "test" will use the testing dataset. \
RAD: \
g++ -std=c++11 rad.cpp -o RAD \
To run in terminal: ./RAD (input arg: "train" or "test") \
HJPD: \
g++ -std=c++11 hjpd.cpp -o HJPD  \
To run in terminal: ./HJPD (input arg: "train or "test")\
HOD: \
g++ -std=c++11 hod.cpp -o HOD \
To run in terminal: ./HOD (input arg: "train" or "test") \
\
\
Implementation Information:
For detected NAN values, the whole frame containing a single NAN value is deleted. \
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
The histogram uses the distance's magnitude as the real-value count and then is normalized by the total distance traveled in the joint's trajectory. \
