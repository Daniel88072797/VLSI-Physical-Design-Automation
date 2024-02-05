#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"
#include <cstdlib>
#include <iostream>

GlobalPlacer::GlobalPlacer(wrapper::Placement &placement)
    : _placement(placement)
{
}

void GlobalPlacer::randomPlace() //can be optimize
{
    
    double coreWidth = _placement.boundryRight() - _placement.boundryLeft();
    double coreHeight = _placement.boundryTop() - _placement.boundryBottom();
    for (size_t i = 0; i < _placement.numModules(); ++i)
    {
        if (_placement.module(i).isFixed())
            continue;

        double width = _placement.module(i).width();
        double height = _placement.module(i).height();
        double x = rand() % static_cast<int>(coreWidth - width) + _placement.boundryLeft();
        double y = rand() % static_cast<int>(coreHeight - height) + _placement.boundryBottom();
        _placement.module(i).setPosition(x, y);
    }
}

void GlobalPlacer::centerPlace() //can be optimize
{
    

    double coreCenter_X = (_placement.boundryRight() + _placement.boundryLeft()) / 2;
    double coreCenter_Y = (_placement.boundryTop() + _placement.boundryBottom()) / 2;
    for (size_t i = 0; i < _placement.numModules(); ++i)
    {
        if (_placement.module(i).isFixed())
            continue;
        _placement.module(i).setPosition(coreCenter_X, coreCenter_Y);

    }
}


void GlobalPlacer::place()
{
    ///////////////////////////////////////////////////////////////////
    // The following example is only for analytical methods.
    // if you use other methods, you can skip and delete it directly.
    //////////////////////////////////////////////////////////////////
    // srand(20);
    srand(20);
    randomPlace();
    
    // centerPlace();

	const unsigned int num_modules = _placement.numModules();
	// unsigned int iters = 2;
	vector<double> x(num_modules * 2);
    for(unsigned int module_id = 0; module_id < num_modules; module_id++){
        x[module_id * 2] = _placement.module(module_id).centerX();
        x[module_id * 2 + 1] = _placement.module(module_id).centerY();
    }

    // beta = alpha / 50000;
    // ef.beta += iter * 1000;
    // const double max_step_size = (_placement.boundryRight() - _placement.boundryLeft()) * 5; //public2 196713404
    // const double max_step_size = (_placement.boundryRight() - _placement.boundryLeft()) * 5; //public1 18246571
    // const double max_step_size = (_placement.boundryRight() - _placement.boundryLeft()) * 5; //public3 1130492750
    const double max_step_size = (_placement.boundryRight() - _placement.boundryLeft()) * 5;
    double x_center, y_center, width_half, height_half;

    ExampleFunction ef(_placement);
    // beta = alpha / 50000;
    // unsigned int iteration[] = {100, 10, 5}; //public2
    // unsigned int iteration[] = {100, 5, 5}; //public1
    // unsigned int iteration[] = {100, 5, 5}; //public3
        
    
    NumericalOptimizer no(ef);
    no.setX(x);             // set initial solution
    no.setNumIteration(200); // user-specified parameter
    no.setStepSizeBound(max_step_size); // user-specified parameter
    no.solve();             // Conjugate Gradient solver

    for(unsigned int module_id = 0; module_id < _placement.numModules(); module_id++){

        if (_placement.module(module_id).isFixed())  //if moved: [Error] Illegal Movement! "p1" is a terminal node that should not be moved.
            continue;

        width_half = _placement.module(module_id).width() / 2;
        height_half = _placement.module(module_id).height() / 2;

        x_center = no.x(module_id * 2);
        y_center = no.x(module_id * 2 + 1);

        if(x_center + width_half > _placement.boundryRight()){

            x_center = _placement.boundryRight() - width_half;

        }   
        else if(x_center - width_half < _placement.boundryLeft()){

            x_center = _placement.boundryLeft() + width_half;

        }
        else if(y_center + height_half > _placement.boundryTop()){

            y_center = _placement.boundryTop() - height_half;

        }  
        else if(y_center - height_half < _placement.boundryBottom()){

            y_center = _placement.boundryBottom() + height_half;

        }

        x[module_id * 2] = x_center;
        x[module_id * 2 + 1] = y_center;

        _placement.module(module_id).setCenterPosition(x_center , y_center);
    }
    

/*-------------------------------------------------------------------------------------------------------------------------------*/
    
    // cout << "Current solution:\n";
    // for (unsigned i = 0; i < no.dimension(); i++)
    // {
    //     cout << "x[" << i << "] = " << no.x(i) << "\n";
    // }
    // cout << "Objective: " << no.objective() << "\n";

    ////////////////////////////////////////////////////////////////
    // An example of random placement implemented by TA.
    // If you want to use it, please uncomment the folllwing 1 line.
    

    /* @@@ TODO
     * 1. Understand above example and modify ExampleFunction.cpp to implement the analytical placement (v)
     * 2. You can choose LSE or WA as the wirelength model, the former is easier to calculate the gradient (v)
     * 3. For the bin density model, you could refer to the lecture notes (v)
     * 4. You should first calculate the form of wirelength model and bin density model and the forms of their gradients ON YOUR OWN (v)
     * 5. Replace the value of f in evaluateF() by the form like "f = alpha*WL() + beta*BinDensity()" (v)
     * 6. Replace the form of g[] in evaluateG() by the form like "g = grad(WL()) + grad(BinDensity())" (v)
     * 7. Set the initial vector x in place(), set step size, set #iteration, and call the solver like above example
     * */
}
