#include "ExampleFunction.h"


// minimize 3*x^2 + 2*x*y + 2*y^2 + 7

ExampleFunction::ExampleFunction(wrapper::Placement &placement) : _placement(placement)
{
	cout << "numModules:" << _placement.numModules() << endl;
	if(_placement.numModules() == 12028){

		row_bin_num = 7;  //public1

	}
	else{

		row_bin_num = 8; //else

	}

	core_width = _placement.boundryRight() - _placement.boundryLeft();
	core_height = _placement.boundryTop() - _placement.boundryBottom();
	
	alpha  = (core_width + core_height)/100;
	cout << "core width:" << core_width << "core height:" << core_height << endl;
 
    beta = alpha / 100;
	cout << _placement.boundryLeft() << " " <<  _placement.boundryBottom() << endl;

	init_bin();
	avg_density = get_avg_density(); //get average density

}

void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g)
{
	g = vector<double>(g.size(), 0);
	f = calLSE(x, g) + calBinDensity(x,g);
}

void ExampleFunction::evaluateF(const vector<double> &x, double &f)
{
	f = calLSE(x) + calBinDensity(x);
}

unsigned ExampleFunction::dimension()
{
    return 2*_placement.numModules(); // num_blocks*2
    // each two dimension represent the X and Y dimensions of each block
}

double ExampleFunction::get_avg_density()
{
	double bin_density = 0;
	
	for ( int i = 0 ; i < row_bin_num ; i++){
		for (int j = 0; j < row_bin_num ; j++){
			
			for (unsigned module_id = 0 ; module_id < _placement.numModules() ; module_id++){

				double module_width = _placement.module(module_id).width();
				double module_height= _placement.module(module_id).height();

				double theta_X;
				double theta_Y;
				
				double dx_abs = abs( _placement.module(module_id).centerX() - bins_X[i][j]);
				if ( dx_abs <=  (module_width / 2 + bin_width / 2) && dx_abs >= 0 ){

					double a = 4 / ((bin_width + module_width) * (2 * bin_width + module_width));
					theta_X = 1 - a * dx_abs * dx_abs;

				}
				else if ( dx_abs <= bin_width+module_width / 2 ){

					double b = 4 / (bin_width * (2 * bin_width + module_width));
					theta_X = b * ( dx_abs - bin_width - module_width / 2) * ( dx_abs - bin_width - module_width / 2);

				}
				else{

					theta_X = 0;

				}
				
				double dy_abs = abs( _placement.module(module_id).centerY() - bins_Y[i][j]);
				if ( dy_abs <= ( bin_height/2 + module_height) && dy_abs >=0 ){

					double a = 4/((bin_height + module_height) * ( 2 * bin_height + module_height));
					theta_Y = 1 - a * dy_abs * dy_abs;

				}
				else if ( dy_abs <= bin_height + module_height/2 ){

					double b = 4 / (bin_height*(2 * bin_height + module_height));
					theta_Y = b * ( dy_abs - bin_height - module_height / 2) * ( dy_abs - bin_height - module_height / 2);

				}
				else{

					theta_Y = 0;

				}
				bin_density += theta_X * theta_Y;
			}
			
		}
		
	}
	bin_density /= row_bin_num * row_bin_num;
	return bin_density;


}


double ExampleFunction::calLSE(const vector<double> &x)
{
    vector<double> exp_x;
    vector<double> exp_negative_x;
    vector<double> exp_y;
    vector<double> exp_negative_y;
    
    
    for(unsigned i = 0; i < _placement.numModules(); i++){
        
        exp_x.push_back( exp(x[ 2 * i ] / alpha) );
        exp_y.push_back( exp(x[ 2 * i + 1 ] / alpha) );

        exp_negative_x.push_back( exp( (-1 * x[ 2 * i ]) / alpha) );
        exp_negative_y.push_back( exp( (-1 * x[ 2 * i + 1 ]) / alpha) );

    }

    double sum_exp_x, sum_exp_y, sum_exp_negative_x, sum_exp_negative_y;
    int pin_num;
    double total_lse = 0;

    for(unsigned net_id = 0; net_id < _placement.numNets(); net_id++){

        sum_exp_x = 0;
        sum_exp_y = 0; 
        sum_exp_negative_x = 0;
        sum_exp_negative_y = 0;
        pin_num = _placement.net(net_id).numPins();

        for(int i = 0; i < pin_num; i++){

            int module_id = _placement.net(net_id).pin(i).moduleId();
            sum_exp_x += exp_x[module_id];
            sum_exp_y += exp_y[module_id];
            sum_exp_negative_x += exp_negative_x[module_id];
            sum_exp_negative_y += exp_negative_y[module_id];

        }

        total_lse += log(sum_exp_x) + log(sum_exp_y) + log(sum_exp_negative_x) + log(sum_exp_negative_y);

    }
    
    return alpha * total_lse;
}

double ExampleFunction::calLSE(const vector<double> &x, vector<double> &g)
{
    vector<double> exp_x, exp_negative_x, exp_y, exp_negative_y;
    
    for(unsigned i = 0; i < _placement.numModules(); i++){
        
        exp_x.push_back( exp(x[ 2 * i ] / alpha) );
        exp_y.push_back( exp(x[ 2 * i + 1 ] / alpha) );

        exp_negative_x.push_back( exp( (-1 * x[ 2 * i ]) / alpha) );
        exp_negative_y.push_back( exp( (-1 * x[ 2 * i + 1 ]) / alpha) );

    }

    double sum_exp_x, sum_exp_y, sum_exp_negative_x, sum_exp_negative_y;
    int pin_num;
    double total_lse = 0;

    for(unsigned net_id = 0; net_id < _placement.numNets(); net_id++){

        sum_exp_x = 0;
        sum_exp_y = 0; 
        sum_exp_negative_x = 0;
        sum_exp_negative_y = 0;
        pin_num = _placement.net(net_id).numPins();

        for(int i = 0; i < pin_num; i++){

            int module_id = _placement.net(net_id).pin(i).moduleId();
            sum_exp_x += exp_x[module_id];
            sum_exp_y += exp_y[module_id];
            sum_exp_negative_x += exp_negative_x[module_id];
            sum_exp_negative_y += exp_negative_y[module_id];

        }

        total_lse += log(sum_exp_x) + log(sum_exp_y) + log(sum_exp_negative_x) + log(sum_exp_negative_y);

        for (int i = 0; i < pin_num; i++) {

            int module_id = _placement.net(net_id).pin(i).moduleId();

            // if(!_placement.module(module_id).isFixed()){

			g[module_id * 2] += exp_x[module_id] / sum_exp_x;
			g[module_id * 2 + 1] += exp_y[module_id] / sum_exp_y;
			g[module_id * 2] -= exp_negative_x[module_id] / sum_exp_negative_x;
			g[module_id * 2 + 1] -= exp_negative_y[module_id] / sum_exp_negative_y;

            // }
        }

    }
    
    cout << "total_lse:" << total_lse << endl;
    return alpha * total_lse;
}


void ExampleFunction::init_bin(){

    // Bin bins[row_bin_num][row_bin_num];
	bin_width = (double)core_width/row_bin_num;
	bin_height = (double)core_height/row_bin_num;


	for ( int i = 0 ; i < row_bin_num ; i++){
		for ( int j = 0 ; j < row_bin_num ; j++ ){
			bins_X[i][j] = _placement.boundryLeft() + i * bin_width + 0.5 * bin_width;
			bins_Y[i][j] = _placement.boundryBottom() + j * bin_height + 0.5 * bin_height;
		}
	}
    

}
double ExampleFunction::calBinDensity (const vector<double> &x) //x儲存中心座標 
{
	double bin_density = 0;
	double total = 0;
	
	for ( int i = 0 ; i < row_bin_num ; i++){
		for (int j = 0; j < row_bin_num ; j++){
			bin_density = 0;
			for (unsigned module_id = 0 ; module_id < _placement.numModules() ; module_id++){

				double module_width = _placement.module(module_id).width();
				double module_height= _placement.module(module_id).height();

				double theta_X;
				double theta_Y;
				
				double dx = x[2 * module_id] - bins_X[i][j];
				double dx_abs = abs( dx);
				double dy = x[2 * module_id + 1] - bins_Y[i][j];
				double dy_abs = abs(dy);

				if ( dx_abs <=  (module_width / 2 + bin_width / 2) && dx_abs >= 0 ){

					double a = 4 / ((bin_width + module_width) * (2 * bin_width + module_width));
					theta_X = 1 - a * dx_abs * dx_abs;

				}
				else if ( dx_abs <= bin_width+module_width / 2 ){

					double b = 4 / (bin_width * (2 * bin_width + module_width));
					theta_X = b * ( dx_abs - bin_width - module_width / 2) * ( dx_abs - bin_width - module_width / 2);

				}
				else{

					theta_X = 0;

				}
				
				if ( dy_abs <= ( bin_height/2 + module_height) && dy_abs >=0 ){

					double a = 4/((bin_height + module_height) * ( 2 * bin_height + module_height));
					theta_Y = 1 - a * dy_abs * dy_abs;

				}
				else if ( dy_abs <= bin_height + module_height/2 ){

					double b = 4 / (bin_height*(2 * bin_height + module_height));
					theta_Y = b * ( dy_abs - bin_height - module_height / 2) * ( dy_abs - bin_height - module_height / 2);

				}
				else{

					theta_Y = 0;

				}
				bin_density += theta_X * theta_Y;
			}

			total += (bin_density - avg_density) * (bin_density - avg_density);  
		}

	}
	return beta * total;
}

double ExampleFunction::calBinDensity(const vector<double> &x, vector<double> &g)
{
	double bin_density = 0;
	double total = 0;
	
	for ( int i = 0 ; i < row_bin_num ; i++){
		for (int j = 0; j < row_bin_num ; j++){
			bin_density = 0;
			vector<double> g_buf(g.size(),0);

			for (unsigned module_id = 0 ; module_id < _placement.numModules() ; module_id++){

				double module_width = _placement.module(module_id).width();
				double module_height= _placement.module(module_id).height();

				double theta_X;
				double theta_Y;
				double theta_X_g = 0;
				double theta_Y_g = 0;
				double dx = x[2 * module_id] - bins_X[i][j];
				double dx_abs = abs( dx);
				double dy = x[2 * module_id + 1] - bins_Y[i][j];
				double dy_abs = abs(dy);

				if ( dx_abs <=  (module_width / 2 + bin_width / 2) && dx_abs >= 0 ){

					double a = 4 / ((bin_width + module_width) * (2 * bin_width + module_width));
					theta_X = 1 - a * dx_abs * dx_abs;
					theta_X_g = -2 * a * (dx) * 1;

				}
				else if ( dx_abs <= bin_width + module_width / 2 ){

					double b = 4 / (bin_width * (2 * bin_width + module_width));
					theta_X = b * ( dx_abs - bin_width - module_width / 2) * ( dx_abs - bin_width - module_width / 2);
					if ( dx > 0 ){

						theta_X_g = 2 * b * ( dx_abs - bin_width - module_width/2) * 1; 

					}
					else {

						theta_X_g = 2 * b * ( -dx_abs - bin_width - module_width/2) * (-1);

					}

				}
				else{

					theta_X = 0;
					theta_X_g = 0;

				}
				
				if ( dy_abs <= ( bin_height / 2 + module_height) && dy_abs >= 0 ){

					double a = 4/((bin_height + module_height) * ( 2 * bin_height + module_height));
					theta_Y = 1 - a * dy_abs * dy_abs;
					theta_Y_g = -2 * a * (dy) * 1;

				}
				else if ( dy_abs <= bin_height + module_height/2 ){

					double b = 4 / (bin_height*(2 * bin_height + module_height));
					theta_Y = b * ( dy_abs - bin_height - module_height / 2) * ( dy_abs - bin_height - module_height / 2);
					if ( dy > 0 ){

						theta_Y_g = 2 * b * (dy_abs - bin_height-module_height/2)*1;

					}
					else {

						theta_Y_g = 2 * b * ( -dy_abs-bin_height-module_height/2)*(-1);

					}

				}
				else{

					theta_Y = 0;
					theta_Y_g = 0;

				}
				bin_density += theta_X * theta_Y;
				g_buf[ 2*module_id ] +=  theta_X_g * theta_Y;
				g_buf[ 2*module_id+1 ] +=  theta_X * theta_Y_g;

			}

			total += (bin_density - avg_density) * (bin_density - avg_density);  
			for (unsigned k = 0 ; k < _placement.numModules() ; k++){

				g[ 2*k ] += 2 * beta * (bin_density - avg_density) * g_buf[ 2 * k ];   //chain rule
				g[ 2*k+1 ] += 2 * beta * (bin_density - avg_density) * g_buf[ 2 * k + 1 ];	//chain rule

			}			
		}
		//cout << endl;
	}
	cout << "bin density:" << total << endl;
	return beta * total;
}




/*Example*/

// int main()
// {						// minimize 3*x^2 + 2*x*y + 2*y^2 + 7
// 	ExampleFunction ef; // require to define the object function and gradient function

// 	vector<double> x(2); // solution vector, number of variables
// 	x[0] = 100;			 // initialize the solution vector
// 	x[1] = 100;

// 	NumericalOptimizer no(ef);
// 	no.setX(x);				// set initial solution
// 	no.setNumIteration(35); // user-specified parameter
// 	no.setStepSizeBound(5); // user-specified parameter
// 	no.solve();				// Conjugate Gradient solver

// 	cout << "Current solution:\n";
// 	for (unsigned i = 0; i < no.dimension(); i++)
// 	{
// 		cout << "x[" << i << "] = " << no.x(i) << "\n";
// 	}
// 	cout << "Objective: " << no.objective() << "\n";
// 	////////////////////////////////////////////////////////////////
// 	return 0;
// }
