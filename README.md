
Compilation and Execution Instructions: \
1.) Extract the folder from the tarball into a single location. Ensure that all the files extracted are in the same folder. \
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
