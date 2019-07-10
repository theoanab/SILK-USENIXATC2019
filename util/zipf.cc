#include "zipf.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// 	*
// 	 * Create a zipfian generator for items between min and max (inclusive) for the specified zipfian constant, using the precomputed value of zeta.
// 	 * 
// 	 * @param min The smallest integer to generate in the sequence.
// 	 * @param max The largest integer to generate in the sequence.


long items; //initialized in init_zipf_generator function
long base; //initialized in init_zipf_generator function
double zipfianconstant; //initialized in init_zipf_generator function
double alpha; //initialized in init_zipf_generator function
double zetan; //initialized in init_zipf_generator function
double eta; //initialized in init_zipf_generator function
double theta; //initialized in init_zipf_generator function
double zeta2theta; //initialized in init_zipf_generator function
long countforzeta; //initialized in init_zipf_generator function
long lastVal; //initialized in setLastValue

void init_zipf_generator(long min, long max){
	items = max-min+1;
	base = min;
	zipfianconstant = 0.99;
	theta = zipfianconstant;
	zeta2theta = zeta(0, 2, 0);
	alpha = 1.0/(1.0-theta);
	zetan = zetastatic(0, max-min+1, 0);
	countforzeta = items;
	eta=(1 - pow(2.0/items,1-theta) )/(1-zeta2theta/zetan);

	nextValue();
}


double zeta(long st, long n, double initialsum) {
	countforzeta=n;
	return zetastatic(st,n,initialsum);
}

//initialsum is the value of zeta we are computing incrementally from
double zetastatic(long st, long n, double initialsum){
	double sum=initialsum;
	for (long i=st; i<n; i++){
		sum+=1/(pow(i+1,theta));
	}				
	return sum;
}

long nextLong(long itemcount){
	//from "Quickly Generating Billion-Record Synthetic Databases", Jim Gray et al, SIGMOD 1994
	if (itemcount!=countforzeta){
		if (itemcount>countforzeta){
			printf("WARNING: Incrementally recomputing Zipfian distribtion. (itemcount= %ld; countforzeta= %ld)", itemcount, countforzeta);
			//we have added more items. can compute zetan incrementally, which is cheaper
			zetan = zeta(countforzeta,itemcount,zetan);
			eta = ( 1 - pow(2.0/items,1-theta) ) / (1-zeta2theta/zetan);
		} 
	}
	
	double u = (double)rand() / ((double)RAND_MAX);
	double uz=u*zetan;
	if (uz < 1.0){
		return base;
	}
	
	if (uz<1.0 + pow(0.5,theta)) {
		return base + 1;
	}
	long ret = base + (long)((itemcount) * pow(eta*u - eta + 1, alpha));
	setLastValue(ret);
	return ret;
}

long nextValue() {
	return nextLong(items);
}

void setLastValue(long val){
	lastVal = val;
}

