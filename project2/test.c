#include <stdio.h>

int main(void)
{
	int t0, t1, t2, t3, s0, s1, s2, s3, a1, ra;
	
	// main
	s1 = 0;
	s2 = 0;
	t0 = 100;
	t1 = 200;
	t2 = 0;

	// lab1
	LAB:
	printf("LAB\n");
	t3 = 0;

	// lab2
	do
	{
		s1 += 1;
		t3 += 1;
		t1 = t1 | 0;
	} while(t3 != t0);
	printf("t0: %d, t1: %d, t2: %d, t3: %d, s1: %d, s2: %d, s3: %d\n", t0, t1,t2, t3, s1, s2, s3);

	// lab3
	do
	{
		s2 += 2;
		t3 += 1;
		s2 = s1 << 1;
		s1 = s2 >> 1;
		s3 = s1 & s2;
	} while (t3 != t1);
	printf("t0: %d, t1: %d, t2: %d, t3: %d, s1: %d, s2: %d, s3: %d\n", t0, t1,t2, t3, s1, s2, s3);

	// lab4
	a1 += ra;
	s0 = ~(s1|s2);
	//if (t2 != t0) goto LAB;
	//else 	printf("t0: %d, t1: %d, t2: %d, t3: %d, s0: %d, s1: %d, s2: %d, s3: %d\n", t0, t1,t2, t3, s0, s1, s2, s3);

	
	
	return 0;
}
