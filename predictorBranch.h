#ifndef PREDICTOR_H
#define PREDICTOR_H
#include <stdint.h>
#include <math.h>

typedef struct globalRegister
{
    unsigned long int gRegister = 0;

}globalRegister;

class predictionCounterTable
{
    public:
	int sets;
	int* pCounter;

	predictionCounterTable(int M2)
	{
	    sets = (int)(pow(2,M2));
	    pCounter = new int[sets]();

	    for (int i=0; i<sets; i++)
	    {
		pCounter[i] = 2;
	    }
	}
};

class chooserTable
{
	public:
	int sets;
	int *pChooser;
	
	chooserTable(int K)
	{
		sets = (int)(pow(2, K));
		pChooser = new int[sets]();
		
		for (int i=0; i<sets; i++)
		{
			pChooser[i] = 1;
		}
	}
};

#endif
