#include "../ml_optimiser_mpi.h"
#include "../backprojector.h"

void maskdata(MultidimArray<RFLOAT> &V);
void targetdata(MultidimArray<RFLOAT> V);



void decenter(MultidimArray<float> &Min, MultidimArray<float> &Mout, int my_rmax2);
void testdecenter(MultidimArray<RFLOAT> V);
void decenter2d(MultidimArray<float> &Min, MultidimArray<float> &Mout, int my_rmax2);
