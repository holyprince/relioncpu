
#include "../ml_optimiser_mpi.h"
#include "datalayout.h"


void setvalue(MlWsumModel &summodel,int ori_size,int datavalue)
{

	int nr_groups=summodel.sigma2_noise.size();
    int nr_classes_bodies = summodel.BPref.size();
    int nr_classes = summodel.pdf_class.size();
    int spectral_size = (ori_size / 2) + 1;

	summodel.LL=1;
	summodel.ave_Pmax=2;
	summodel.sigma2_offset=3;
	summodel.avg_norm_correction=4;
	summodel.sigma2_rot=5;
	summodel.sigma2_tilt=6;
	summodel.sigma2_psi=7;

    for (int igroup = 0; igroup < nr_groups; igroup++)
    {
    	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(summodel.sigma2_noise[igroup])
        {
    		DIRECT_MULTIDIM_ELEM(summodel.sigma2_noise[igroup], n) = datavalue;
        }
        FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(summodel.wsum_signal_product_spectra[igroup])
        {
        	DIRECT_MULTIDIM_ELEM(summodel.wsum_signal_product_spectra[igroup], n) = datavalue;
        }
        FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(summodel.wsum_reference_power_spectra[igroup])
        {
            DIRECT_MULTIDIM_ELEM(summodel.wsum_reference_power_spectra[igroup], n)= datavalue;
        }
        summodel.sumw_group[igroup]=igroup;
    }

    for (int iclass = 0; iclass < nr_classes_bodies; iclass++)
    {
    	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(summodel.BPref[iclass].data)
        {
            (DIRECT_MULTIDIM_ELEM(summodel.BPref[iclass].data, n)).real= datavalue;
            (DIRECT_MULTIDIM_ELEM(summodel.BPref[iclass].data, n)).imag = datavalue;
            DIRECT_MULTIDIM_ELEM(summodel.BPref[iclass].weight, n)= datavalue;
        }

        FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(summodel.pdf_direction[iclass])
        {
            DIRECT_MULTIDIM_ELEM(summodel.pdf_direction[iclass], n) = datavalue;
        }
    }

    for (int iclass = 0; iclass < nr_classes; iclass++)
    {
    	summodel.pdf_class[iclass]= iclass;
    }
}
void initwsmmodel(MlWsumModel &summodel, int cur_size,int ori_size,int nr_groups)
{
	MultidimArray<RFLOAT > aux;
    aux.initZeros(ori_size / 2 + 1);
	int ref_dim=3;
	summodel.ori_size=ori_size;
	summodel.ref_dim=ref_dim;
	summodel.current_size=cur_size;
	summodel.nr_classes=1;
	summodel.nr_bodies=1;
	summodel.nr_groups=nr_groups;
	summodel.nr_directions=10000;
	summodel.pdf_direction.resize(1);
	summodel.pdf_direction[0].initZeros(summodel.nr_directions);
	summodel.sigma2_noise.resize(nr_groups, aux);

	BackProjector BP(ori_size,ref_dim,"C1");
	summodel.BPref.clear();
	summodel.BPref.resize(summodel.nr_classes * summodel.nr_bodies, BP); // also set multiple bodies
	BP.clear();
	summodel.pdf_class.resize(summodel.nr_classes, 1./(RFLOAT)summodel.nr_classes);
	summodel.sumw_group.resize(nr_groups);
	summodel.wsum_signal_product_spectra.resize(nr_groups, aux);
	summodel.wsum_reference_power_spectra.resize(nr_groups, aux);
	//printf("before : %f \n",summodel.LL);
	summodel.initZeros();
	//printf("after : %f \n",summodel.LL);
	//printf("BPrefsize : %d \n",summodel.BPref[0].getSize());
	aux.clear();

}

void testmpicombine(int argc, char **argv)
{
	MlOptimiserMpi optimiser;

	optimiser.node = new MpiNode(argc, argv);

	//printf("MPI %d and %d ",optimiser.node->rank,optimiser.node->size);

	optimiser.do_split_random_halves=1;
	printMpiNodesMachineNames(*(optimiser.node), 1);
	//optimiser.initialise();
	int ori_size=360;
	int cur_size=34;

	initwsmmodel(optimiser.wsum_model,cur_size,ori_size,1000);
	setvalue(optimiser.wsum_model,ori_size,optimiser.node->rank);

		//printf("before combine : %f rank is %d  \n",optimiser.wsum_model.BPref[0].data.data[0].real,optimiser.node->rank);
	struct timeval tv1,tv2;
	struct timezone tz;
	float time_use;
	gettimeofday (&tv1, &tz);
	//optimiser.combineAllWeightedSums();
	optimiser.combineAllWeightedSumsallreduce();
	gettimeofday (&tv2, &tz);
	time_use=1000 * (tv2.tv_sec-tv1.tv_sec)+ (tv2.tv_usec-tv1.tv_usec)/1000;
	if(optimiser.node->rank==1)
	{
		printf("combinedata : %f ms and process id is %d \n", time_use,optimiser.node->rank) ;
		printf("after combine : %f rank is %d \n",optimiser.wsum_model.BPref[0].data.data[0].real,optimiser.node->rank);
	}

}
void compressmodle(MlWsumModel &model,int cur_size)
{
	int r_max=cur_size/2;
	int padding_factor=2;
	int max_r2= ROUND(r_max * padding_factor) * ROUND(r_max * padding_factor);
	int ydim=model.BPref[0].data.ydim;
	FOR_ALL_ELEMENTS_IN_ARRAY3D(model.BPref[0].data)
	{
		if (k * k + i * i + j * j < max_r2)
		{
			int datacur;
			datacur=model.BPref[0].yoffsetdata[(k-STARTINGZ(model.BPref[0].data))*ydim+(i-STARTINGY(model.BPref[0].data))]+(j-STARTINGX(model.BPref[0].data));
			model.BPref[0].compweight.data[datacur]=1;
		}
	}
}
void testcompresscombine(int argc, char **argv)
{
	MlOptimiserMpi optimiser;

	optimiser.node = new MpiNode(argc, argv);

	//printf("MPI %d and %d ",optimiser.node->rank,optimiser.node->size);

	optimiser.do_split_random_halves=1;
	printMpiNodesMachineNames(*(optimiser.node), 1);
	//optimiser.initialise();
	int ori_size=500;
	int cur_size=488;

	initwsmmodel(optimiser.wsum_model,cur_size,ori_size,1000);
	setvalue(optimiser.wsum_model,ori_size,optimiser.node->rank);
	compressmodle(optimiser.wsum_model,cur_size);

	struct timeval tv1,tv2;
	struct timezone tz;
	float time_use;
	gettimeofday (&tv1, &tz);
	//optimiser.combineAllWeightedSums();
	//optimiser.combineAllWeightedSumscompressdata();
	optimiser.combineAllWeightedSumsallreducewithcompress();
	gettimeofday (&tv2, &tz);
	time_use=1000 * (tv2.tv_sec-tv1.tv_sec)+ (tv2.tv_usec-tv1.tv_usec)/1000;
	if(optimiser.node->rank==1)
	{
		printf("combinedata : %f ms and process id is %d \n", time_use,optimiser.node->rank) ;
		printf("after combine : %f rank is %d \n",optimiser.wsum_model.BPref[0].data.data[0].real,optimiser.node->rank);
	}

}
int main(int argc, char **argv)
//int testmain(int argc, char **argv)
{
/*	MlOptimiser optimiser;

	int ori_size=360;
	int cur_size=34;

	initwsmmodel(optimiser.wsum_model,cur_size,ori_size,1000);
	setvalue(optimiser.wsum_model,ori_size,100);

	maskdata(optimiser.wsum_model.BPref[0].weight);

	targetdata(optimiser.wsum_model.BPref[0].weight);
	//testdecenter(optimiser.wsum_model.BPref[0].weight);
*/
	testmpicombine(argc,argv);
//	testcompresscombine(argc,argv);


}

//method1 : use origin read method
//method2 : self input method



