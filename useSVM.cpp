#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include "svm.h"
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

void exit_input_error(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

int print_null(const char *s,...) {return 0;}

static int (*info)(const char *fmt,...) = &printf;
struct svm_parameter param;		// set by parse_command_line
struct svm_problem prob;		// set by read_problem
struct svm_model *model;
struct svm_node *x_space;
struct svm_node *x;
int cross_validation;

static char *line = NULL;
static int max_line_len;

int max_nr_attr = 64;
int predict_probability=0;


// Prototypes
void read_problem(const char *filename);
void predict(FILE *input);
static char* readline(FILE *input);

int main(int argc, char **argv)
{
	char input_file_name[1024];
    FILE *input_training;

	const char *error_msg;

    // default values
    // SVM Type
	param.svm_type = C_SVC;     // C_SVC:       C-SVM classification
                                // NU_SVC:      nu-SVM classification
                                // ONE_CLASS:   one-class-SVM
                                // EPSILON_SVR:	epsilon-SVM regression
                                // NU_SVR:		nu-SVM regression
	// Kernel Type
    param.kernel_type = RBF;    // LINEAR:	    u'*v  
                                // POLY:	    (gamma*u'*v + coef0)^degree
                                // RBF:	        exp(-gamma*|u-v|^2)
                                // SIGMOID:	    tanh(gamma*u'*v + coef0)
                                // PRECOMPUTED: kernel values in training_set_file
	
    param.degree = 3;           
	param.gamma = 0;	
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	cross_validation = 0;

    // Input Args
    param.C = atof(argv[1]);                // First value is C value for training model
    param.gamma = atof(argv[2]);            // Second value is gamma value for training model
    strcpy(input_file_name, argv[3]);       // Third value is input file name for training model
    input_training = fopen(argv[4], "r");   // Fourth value is testing data file name
    if(input_training == NULL)
	{
        std::cout << "Error: Cannot open input training file" << std::endl;
		exit(1);
	}         


	read_problem(input_file_name);
	error_msg = svm_check_parameter(&prob,&param);
	if(error_msg)
	{
		fprintf(stderr,"ERROR: %s\n",error_msg);
		exit(1);
	}

    model = svm_train(&prob,&param);

    std::cout << "#############################" << std::endl;
    std::cout << "#------Training Done--------#" << std::endl;
    std::cout << "#------Model Created--------#" << std::endl;
    std::cout << "#############################" << std::endl;


	x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
	predict(input_training);

    // Cleanup after operations
    svm_free_and_destroy_model(&model);
	svm_destroy_param(&param);
	free(prob.y);
	free(prob.x);
	free(x_space);
	free(line);

	return 0;
}


void read_problem(const char *filename)
{
	int max_index, inst_max_index, i;
	size_t elements, j;
	FILE *fp = fopen(filename,"r");
	char *endptr;
	char *idx, *val, *label;

	if(fp == NULL)
	{
		fprintf(stderr,"can't open input file %s\n",filename);
		exit(1);
	}

	prob.l = 0;
	elements = 0;

	max_line_len = 1024;
	line = Malloc(char,max_line_len);
	while(readline(fp)!=NULL)
	{
		char *p = strtok(line," \t"); // label

		// features
		while(1)
		{
			p = strtok(NULL," \t");
			if(p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
				break;
			++elements;
		}
		++elements;
		++prob.l;
	}
	rewind(fp);

	prob.y = Malloc(double,prob.l);
	prob.x = Malloc(struct svm_node *,prob.l);
	x_space = Malloc(struct svm_node,elements);

	max_index = 0;
	j=0;
	for(i=0;i<prob.l;i++)
	{
		inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
		readline(fp);
		prob.x[i] = &x_space[j];
		label = strtok(line," \t\n");
		if(label == NULL) // empty line
			exit_input_error(i+1);

		prob.y[i] = strtod(label,&endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_error(i+1);

		while(1)
		{
			idx = strtok(NULL,":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;

			errno = 0;
			x_space[j].index = (int) strtol(idx,&endptr,10);
			if(endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
				exit_input_error(i+1);
			else
				inst_max_index = x_space[j].index;

			errno = 0;
			x_space[j].value = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(i+1);

			++j;
		}

		if(inst_max_index > max_index)
			max_index = inst_max_index;
		x_space[j++].index = -1;
	}

	if(param.gamma == 0 && max_index > 0)
		param.gamma = 1.0/max_index;

	if(param.kernel_type == PRECOMPUTED)
		for(i=0;i<prob.l;i++)
		{
			if (prob.x[i][0].index != 0)
			{
				fprintf(stderr,"Wrong input format: first column must be 0:sample_serial_number\n");
				exit(1);
			}
			if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
			{
				fprintf(stderr,"Wrong input format: sample_serial_number out of range\n");
				exit(1);
			}
		}

	fclose(fp);
}

static char* readline(FILE *input)
{
	int len;

	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

void predict(FILE *input)
{
	int correct = 0;
	int total = 0;
	double error = 0;
	double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

    std::vector<std::vector<int>> confMat(6, std::vector<int>(6, 0));  // 2D Matrix initialized to all 0
    std::vector<int> actNum = {8, 10, 12, 13, 15, 16};



	int svm_type=svm_get_svm_type(model);
	int nr_class=svm_get_nr_class(model);
	double *prob_estimates=NULL;
	int j;


	max_line_len = 1024;
	line = (char *)malloc(max_line_len*sizeof(char));
	while(readline(input) != NULL)
	{
		int i = 0;
		double target_label, predict_label;
		char *idx, *val, *label, *endptr;
		int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

		label = strtok(line," \t\n");

		target_label = strtod(label,&endptr);



		while(1)
		{
			if(i>=max_nr_attr-1)	// need one more for index = -1
			{
				max_nr_attr *= 2;
				x = (struct svm_node *) realloc(x,max_nr_attr*sizeof(struct svm_node));
			}

			idx = strtok(NULL,":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;
			errno = 0;
			x[i].index = (int) strtol(idx,&endptr,10);

            inst_max_index = x[i].index;

			errno = 0;
			x[i].value = strtod(val,&endptr);

			++i;
		}
		x[i].index = -1;

        predict_label = svm_predict(model,x);

        // Indices for populating Confusion Matrix
        int actualIndex = 0;
        int svmIndex = 0;
        for (int i = 0; i < actNum.size(); i++){    // Finds indices of svm classifier number
            if (predict_label == actNum[svmIndex]) {break;}
            else {svmIndex++;}
        }
        for (int i = 0; i < actNum.size(); i++){    // Finds indices of actual activity number
            if (target_label == actNum[actualIndex]) {break;}
            else {actualIndex++;}
        }
        confMat[actualIndex][svmIndex]++;   // Increment value in confusion matrix

		if(predict_label == target_label)
			++correct;
		error += (predict_label-target_label)*(predict_label-target_label);
		sump += predict_label;
		sumt += target_label;
		sumpp += predict_label*predict_label;
		sumtt += target_label*target_label;
		sumpt += predict_label*target_label;
		++total;
	}

    // Display Confusion Matrix
    std::cout << "######################" << std::endl;
    std::cout << "#--Confusion Matrix--#" << std::endl;
    std::cout << "######################" << std::endl;
    std::cout << "\t";
    for (int i = 0; i < actNum.size(); i++) {std::cout << actNum[i] << "\t";}
    std::cout << std::endl;
    for (int i = 0; i < confMat.size(); i++) {
        std::cout << actNum[i] << "\t";
        for (int j = 0; j < confMat[i].size(); j++) {
            std::cout << confMat[i][j] << "\t";
        }
        std::cout << std::endl;
    }

    // Display Accuracy
    std::cout << "######################" << std::endl;
    std::cout << "#------Accuracy------#" << std::endl;
    std::cout << "######################" << std::endl;
    info("Accuracy = %g%% (%d/%d) (classification)\n",
        (double)correct/total*100,correct,total);

}