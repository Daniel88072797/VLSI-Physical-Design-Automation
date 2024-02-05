#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include "NumericalOptimizerInterface.h"
#include "Wrapper.hpp"
#include <cmath>


class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(wrapper::Placement &placement);
    double calBinDensity(const vector<double> &x);
    double calLSE(const vector<double> &x);
    double calBinDensity(const vector<double> &x, vector<double> &g);
    double calLSE(const vector<double> &x, vector<double> &g);
    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    unsigned dimension();
    void init_bin();
    double get_avg_density();
    double beta;
    
private:
    double bin_width;
    double bin_height;
    double core_width;
    double core_height;
    double alpha; //smooth parameter
    double avg_density;
    int NumOfModule;
    int row_bin_num;


    // static const unsigned int row_bin_num = 8;
    // double BinCenter_x[8][8];
	// double BinCenter_y[8][8];
    double bins_X[10][10];
    double bins_Y[10][10];

    wrapper::Placement &_placement;
};

#endif // EXAMPLEFUNCTION_H
