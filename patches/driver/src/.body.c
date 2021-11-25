#define S1(fk0,fk1,i,j) A[i][j] += B[i]; 

#define S2(fk0,fk1,i,j) A[(i - 1)][j] += B[(i + M)]; 

	 register int lbv, ubv, lb, ub, lb1, ub1, lb2, ub2;
