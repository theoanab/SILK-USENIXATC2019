extern long items; //initialized in init_zipf_generator function
extern long base; //initialized in init_zipf_generator function
extern double zipfianconstant; //initialized in init_zipf_generator function
extern double alpha; //initialized in init_zipf_generator function
extern double zetan; //initialized in init_zipf_generator function
extern double eta; //initialized in init_zipf_generator function
extern double theta; //initialized in init_zipf_generator function
extern double zeta2theta; //initialized in init_zipf_generator function
extern long countforzeta; //initialized in init_zipf_generator function
extern long lastVal; //initialized in setLastValue

void init_zipf_generator(long min, long max);
double zeta(long st, long n, double initialsum);
double zetastatic(long st, long n, double initialsum);
long nextLong(long itemcount);
long nextValue();
void setLastValue(long val);