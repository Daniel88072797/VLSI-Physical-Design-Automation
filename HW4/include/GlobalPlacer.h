#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "Wrapper.hpp"

class GlobalPlacer
{
public:
    GlobalPlacer(wrapper::Placement &placement);
    void centerPlace();
    void randomPlace(); // An example of random placement implemented by TA
    void place();

private:
    wrapper::Placement &_placement;
};

#endif // GLOBALPLACER_H
