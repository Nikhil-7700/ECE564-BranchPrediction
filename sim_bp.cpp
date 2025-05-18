#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sim_bp.h"
#include "predictorBranch.h"

unsigned long int predictionCount = 0;		// Number of Predictions
unsigned long int mispredictionCount = 0;	// Number of MisPredictions
float mispredictionRate = 0.0;				// Misprediction Rate

void biModalBranchPredictor(unsigned long int indexBits, predictionCounterTable* counter, unsigned long int address, char actualOutcome)
{
	predictionCount += 1;

	unsigned int ProgCounter = ((address >> 2) & (counter->sets-1));
	unsigned int predictionOutcome = counter->pCounter[ProgCounter];

	bool branchTaken = false; // 1: Taken Branch; 0: not-Taken Branch
	
	if (predictionOutcome >= 2)
		branchTaken = true;
	// PredictionOutcome == 0 OR 1: Branch is Not Taken
	// PredictionOutcome == 2 OR 3: Branch is Taken

	bool actualOutcomeTaken = false;
	// actualOutcomeTrue == 0: Actual Outcome Not Taken
	// actualOutcomeTrue == 1: Actual Outcome Taken
	
	if (actualOutcome == 't')
		actualOutcomeTaken = true;

	if (branchTaken != actualOutcomeTaken)
		mispredictionCount += 1;

	if (actualOutcomeTaken)
	{
		if (counter->pCounter[ProgCounter] != 3)
		{
			counter->pCounter[ProgCounter] += 1;
			// Increment the Counter Value by 1 when taken
		}
	}
	else
	{
		if (counter->pCounter[ProgCounter] != 0)
		{
			counter->pCounter[ProgCounter] -= 1;
			// Decrement the Counter Value by 1 when not taken
        }
	}
	
}

void gShareBranchPredictor(unsigned long int pcBits, unsigned long int regBits, globalRegister* gReg, predictionCounterTable* counter, unsigned long int address, char actualOutcome)
{
	predictionCount += 1;
	
	int nBits = ((int)(pow(2, regBits)));
	int gloablHistoryIndex = ((gReg->gRegister) & (nBits - 1));
	
	unsigned int ProgCounter = ((address >> 2) & (counter->sets - 1));
	int diffMN = pcBits - regBits;						// m - n
	int powMN = ((int)(pow(2,diffMN)));					// 2**(m-n)
	int indexL = (ProgCounter) & (powMN - 1);		
	int tag = (ProgCounter >> diffMN);
	int indexU = gloablHistoryIndex ^ tag;				// XORed value
	
	int index = (indexU << diffMN) | indexL;			// Index Value into Prediction Table
	
	unsigned int predictionOutcome = counter->pCounter[index];
	
	bool branchTaken = false; // 1: Taken Branch; 0: not-Taken Branch
	
	if (predictionOutcome >= 2)
		branchTaken = true;
	// PredictionOutcome == 0 OR 1: Branch is Not Taken
	// PredictionOutcome == 2 OR 3: Branch is Taken
	
	bool actualOutcomeTaken = false;
	// actualOutcomeTrue == 0: Actual Outcome Not Taken
	// actualOutcomeTrue == 1: Actual Outcome Taken
	
	if (actualOutcome == 't')
		actualOutcomeTaken = true;

	if (branchTaken != actualOutcomeTaken)
		mispredictionCount += 1;
	
	if(actualOutcomeTaken)
    {
        if(counter->pCounter[index] != 3)
        {
            counter->pCounter[index] += 1;
			// Increment the Counter Value by 1 when taken
        }
        //Update the global history gRegister
        gReg->gRegister  = (gReg->gRegister >> 1) | (1 << (regBits-1));
    }

    else
    {
        if(counter->pCounter[index] > 0)
        {
            counter->pCounter[index] -= 1;
			// Decrement the Counter Value by 1 when not taken
        }
		//Update the global history gRegister
        gReg->gRegister  = (gReg->gRegister >> 1) | (0 << (regBits-1));
    }
}

void hybridBranchPredictor(unsigned long int pcBitsC, unsigned long int pcBitsG, unsigned long int regBits, unsigned long int pcBitsB, globalRegister* gReg, predictionCounterTable* counterGShare, predictionCounterTable* counterBiModal, chooserTable *chooser, unsigned long int address, char actualOutcome)
{
	predictionCount += 1;
	
	// GSHARE BRANCH PREDICTOR OUTCOME
	int nBits = ((int)(pow(2, regBits)));
	int gloablHistoryIndex = ((gReg->gRegister) & (nBits - 1));
	
	unsigned int ProgCounterG = ((address >> 2) & (counterGShare->sets - 1));
	int diffMN = pcBitsG - regBits;						// m - n
	int powMN = ((int)(pow(2,diffMN)));					// 2**(m-n)
	int indexL = (ProgCounterG) & (powMN - 1);		
	int tag = (ProgCounterG >> diffMN);
	int indexU = gloablHistoryIndex ^ tag;				// XORed value
	
	int indexG = (indexU << diffMN) | indexL;			// Index Value into Prediction Table
	
	unsigned int predictionOutcomeG = counterGShare->pCounter[indexG];
	
	bool branchTakenG = false; // 1: Taken Branch; 0: not-Taken Branch
	
	if (predictionOutcomeG >= 2)
		branchTakenG = true;
	// PredictionOutcome == 0 OR 1: Branch is Not Taken
	// PredictionOutcome == 2 OR 3: Branch is Taken
	
	// BIMODAL BRANCH PREDICTOR OUTCOME
	unsigned int ProgCounterB = ((address >> 2) & (counterBiModal->sets-1));
	unsigned int predictionOutcomeB = counterBiModal->pCounter[ProgCounterB];
	
	bool branchTakenB = false; // 1: Taken Branch; 0: not-Taken Branch
	
	if (predictionOutcomeB >= 2)
		branchTakenB = true;
	// PredictionOutcome == 0 OR 1: Branch is Not Taken
	// PredictionOutcome == 2 OR 3: Branch is Taken
	
	// HYBRID BRANCH PREDICTION - Choosing the prediction from GShare or Bimodal
	unsigned int ProgCounterChoose = ((address >> 2) & (chooser->sets - 1));
	unsigned int predictionChoose = chooser->pChooser[ProgCounterChoose];
	
	bool actualOutcomeTaken = false;
	// actualOutcomeTrue == 0: Actual Outcome Not Taken
	// actualOutcomeTrue == 1: Actual Outcome Taken
	
	if (actualOutcome == 't')
		actualOutcomeTaken = true;
	
	if (predictionChoose >= 2)
	{
		// Updating the GShare Branch Predictor
		if (branchTakenG != actualOutcomeTaken)
			mispredictionCount += 1;
		
		if (actualOutcomeTaken)
		{
			if(counterGShare->pCounter[indexG] != 3)
				counterGShare->pCounter[indexG] += 1;
			// Increment the Counter Value by 1 when taken
			//Update the global history gRegister
			gReg->gRegister  = (gReg->gRegister >> 1) | (1 << (regBits-1));
		}
		else
		{
			if(counterGShare->pCounter[indexG] != 0)
				counterGShare->pCounter[indexG] -= 1;
			//Update the global history gRegister
			gReg->gRegister  = (gReg->gRegister >> 1) | (0 << (regBits-1));
		}
	}
	
	else
	{
		// Updating the GShare Branch Predictor
		if (branchTakenB != actualOutcomeTaken)
			mispredictionCount += 1;
		
		if (actualOutcomeTaken)
		{
			if(counterBiModal->pCounter[ProgCounterB] != 3)
				counterBiModal->pCounter[ProgCounterB] += 1;
				// Increment the Counter Value by 1 when taken
			
			//Update the global history gRegister - Even though GShare Prediction is not choosen
			gReg->gRegister  = (gReg->gRegister >> 1) | (1 << (regBits-1));
		}
		else
		{
			if(counterBiModal->pCounter[ProgCounterB] != 0)
				counterBiModal->pCounter[ProgCounterB] -= 1;
				// Decrement the Counter Value by 1 when not taken
			
			//Update the global history gRegister - Even though GShare Prediction is not choosen
			gReg->gRegister  = (gReg->gRegister >> 1) | (0 << (regBits-1));
		}
	}
	
	// UPDATING THE CHOOSER TABLE

	if (branchTakenG == branchTakenB)
	{
		// Both the GShare and Bimodal Branch predictions either correct or incorrect
		// continue;
	}
	else if(branchTakenG == actualOutcomeTaken)
	{
		// The GShare Branch predictor gives the correct outcome
		if (chooser->pChooser[ProgCounterChoose] != 3)
			chooser->pChooser[ProgCounterChoose] += 1;
	}
	else
	{
		// The Bimodal Branch predictor gives the correct outcome
		if (chooser->pChooser[ProgCounterChoose] != 0)
			chooser->pChooser[ProgCounterChoose] -= 1;
	}
	
}

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file

    unsigned long int K, M1, M2, N;
    
    predictionCounterTable *counterBiModal;
	predictionCounterTable *counterGShare;
	chooserTable *chooserHybrid;
	
	globalRegister reg;

    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal Branch
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
		M2 = params.M2;
		counterBiModal = new predictionCounterTable(M2);
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare Branch
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
		M1 = params.M1;
		N = params.N;
		counterGShare = new predictionCounterTable(M1);
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
		K = params.K;
		M1 = params.M1;
		N = params.N;
		M2 = params.M2;
		counterBiModal = new predictionCounterTable(M2);
		counterGShare = new predictionCounterTable(M1);
		chooserHybrid = new chooserTable(K);
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        
        outcome = str[0];
        /*************************************
            Add branch predictor code here
        **************************************/
	if (strcmp(params.bp_name, "bimodal") == 0)
	{
	    biModalBranchPredictor(M2, counterBiModal, addr, outcome);
	}
	else if(strcmp(params.bp_name, "gshare") == 0) 
       {
           gShareBranchPredictor(M1, N, &reg, counterGShare, addr, outcome);
       }
	else if(strcmp(params.bp_name, "hybrid") == 0)
       {
           hybridBranchPredictor(K, M1, N, M2, &reg, counterGShare, counterBiModal, chooserHybrid, addr, outcome);
       }
    }
    printf("OUTPUT\n");
    printf("number of predictions:   %d \n",predictionCount);
    printf("number of mispredictions: %d  \n",mispredictionCount);
    mispredictionRate = (((float)(mispredictionCount))/((float)(predictionCount))) * 100;
    printf("misprediction rate:       %.2f%c\n",mispredictionRate,'%');

    if(strcmp(params.bp_name, "bimodal") == 0)
    {
        printf("FINAL BIMODAL CONTENTS");
        for(int i=0; i<counterBiModal->sets; i++)
        {
            printf("\n%d  %d",i,counterBiModal->pCounter[i]);
        }
    }
	else if(strcmp(params.bp_name, "gshare") == 0) 
    {
        printf("FINAL GSHARE CONTENTS");
        for(int i=0; i<counterGShare->sets; i++)
        {
            printf("\n%d  %d",i,counterGShare->pCounter[i]);
        }
    }
	else if(strcmp(params.bp_name, "hybrid") == 0)
    {
        printf("FINAL CHOOSER CONTENTS");
        for(int i=0; i<chooserHybrid->sets; i++)
        {
            printf("\n%d  %d",i,chooserHybrid->pChooser[i]);
        }
        printf("\nFINAL GSHARE CONTENTS");
        for(int i=0; i<counterGShare->sets; i++)
        {
            printf("\n%d  %d",i,counterGShare->pCounter[i]);
        }
        printf("\nFINAL BIMODAL CONTENTS");
        for(int i=0; i<counterBiModal->sets; i++)
        {
            printf("\n%d  %d",i,counterBiModal->pCounter[i]);
        }
    }
    printf("\n");

    return 0;
}
