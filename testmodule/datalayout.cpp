#include "datalayout.h"

void printdata(MultidimArray<RFLOAT> V)
{
	for(int i=0;i<V.zdim;i++)
	{

		if(i==2)
		{
		for(int j=0;j<V.ydim;j++)
		{
			for(int k=0;k<V.xdim;k++)
			{
				printf("%.1f ",V.data[i*V.xdim*V.ydim+j*V.xdim+k]);
				//if(V.data[i*V.xdim*V.ydim+j*V.xdim+k]==1)
				//sumres++;
			}
			printf("\n");
		}

		}
		//printf("\n");
	}
}

void maskdata(MultidimArray<RFLOAT> V)
{


	int r_max=17;
	int padding_factor=2;
	int max_r2= ROUND(r_max * padding_factor) * ROUND(r_max * padding_factor);
/*    for (long int k=STARTINGZ(V); k<=FINISHINGZ(V); k++) \
        for (long int i=STARTINGY(V); i<=FINISHINGY(V); i++) \
            for (long int j=STARTINGX(V); j<=FINISHINGX(V); j++)*/
	int sumres=0;
    FOR_ALL_ELEMENTS_IN_ARRAY3D(V)
	{
		if (k * k + i * i + j * j < max_r2)
		{
			A3D_ELEM(V, k, i, j) = 1.;
			sumres++;
		}
		else
			A3D_ELEM(V, k, i, j) = 0.;
	}
	printf("%d ",sumres);
    printf("\n");



}

void targetdata(MultidimArray<RFLOAT> V)
{
	int zdim=71;
	int ydim=71;
	int xdim=36;
	int *zdata;
	int *ydata;

	int zinit=-(long int)((float) (zdim) / 2.0);
	int yinit=-(long int)((float) (ydim) / 2.0);
	int r_max=17;
	int padding_factor=2;
	int max_r2= ROUND(r_max * padding_factor) * ROUND(r_max * padding_factor);
	//zdata=(int *)malloc(sizeof(int)*zdim);
	ydata=(int *)malloc(sizeof(int)*ydim*zdim);
	memset(ydata,0,sizeof(int)*ydim*zdim);
	for(int iz=0;iz<zdim;iz++)
		for(int jy=0;jy<ydim;jy++)
		{
			int xtemp=max_r2 - (iz+ zinit)*(iz+ zinit) - (jy+yinit)*(jy+yinit);
			if(xtemp<=0)
				ydata[iz*ydim+jy]= 0;
			else
				ydata[iz*ydim+jy]= (int) sqrt(xtemp-0.01)+1;
			//printf("%d \n",ydata[iz*ydim+jy]);
		}
/*	int slice=5;
	for(int i=zdim*slice;i<zdim*(slice+1);i++)
		printf("%d ",ydata[i]+1);
	printf("\n");*/
/*	int sumres=0;
	for(int zslice=0;zslice<zdim;zslice++)
	{

		for(int i=0;i<ydim;i++)
		{
			if(ydata[zslice*ydim+i]!=-1)
				sumres+=ydata[zslice*ydim+i]+1;
		}

	}
	printf("%d ",sumres);*/
	//use this compress data:
	//(x,y,z)->(index) (10,10,10)
	//mei ge z yige fen jiedian
/*	int *slicenum=(int *)malloc(sizeof(int)*zdim);
	for(int zslice=0;zslice<zdim;zslice++)
	{
		slicenum[zslice]=0;
		for(int i=0;i<ydim;i++)
		{
			if(ydata[zslice*ydim+i]!=-1)
				slicenum[zslice]+=ydata[zslice*ydim+i]+1;
		}
	}*/
	//mei ge y
	int *yoffsetdata=(int *)malloc(sizeof(int)*ydim*zdim);

	yoffsetdata[0]=0;
	for(int cur=1;cur<ydim*zdim;cur++)
		yoffsetdata[cur]=yoffsetdata[cur-1]+ydata[cur-1];

	printf("ydata\n");
	for(int i=2*ydim;i<3*ydim;i++)
	{
		printf("%d ",ydata[i]);
	}
	printf("\n");
	printf("yoffsetdata\n");
	for(int i=2*ydim;i<3*ydim;i++)
	{
		printf("%d ",yoffsetdata[i]);
	}


	int sumall=yoffsetdata[ydim*zdim-1]+ydata[ydim*zdim-1];
	printf("sumall check : %d\n",sumall);
	//validation:
	float *compressdata=(float*)malloc(sizeof(float)*sumall);

	FOR_ALL_ELEMENTS_IN_ARRAY3D(V)
	{
		if (k * k + i * i + j * j < max_r2)
		{
			int datacur;
			datacur=yoffsetdata[(k-STARTINGZ(V))*ydim+(i-STARTINGY(V))]+(j-STARTINGX(V));
			compressdata[datacur]=1;
		}
		//else
			//A3D_ELEM(V, k, i, j) = 0.;
	}
	int compressdatasum=0;
	for(int i=0;i<sumall;i++)
		if(compressdata[i]==1)
			compressdatasum++;
	printf("compressdata value : %d\n",compressdatasum);

	V.initZeros(zdim,ydim,xdim);

	//uncompress:
	//method1: copy continus data
	int rawoffset=0;
	int compressoffset=0;
	for(int i=0;i<zdim;i++)
		for(int j=0;j<ydim;j++)
		{
			int curiindex=i*ydim+j;
			memset(V.data+rawoffset,0,xdim*sizeof(float));
			if(ydata[curiindex] !=0 )
				memcpy(V.data+rawoffset,compressdata+compressoffset,ydata[curiindex]*sizeof(float));
			compressoffset += ydata[curiindex];
			rawoffset+=xdim;
		}

	int vailres=0;
	for(int i=0;i<xdim*ydim*zdim;i++)
		if(V.data[i]==1)
			vailres++;
	printf("vailres : %d \n",vailres);
	printdata(V);
	//method2: point-wise copy

	//real apply

	//experiment

}
//y=35,z=5,x=16
