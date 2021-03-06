/***************************************************/
/* Program: shuffle.c                              */
/* Date:    091425PST Jan 1997                     */
/* Name:    Alfred A. Aburto Jr.                   */
/* Email:   aburto@nosc.mil, aburto@cts.com        */
/* Place:   San Diego, CA, USA                     */
/*                                                 */
/* This very simple routine tests a random number  */
/* generator using a card shuffling procedure.     */
/*                                                 */
/* Shuffle.c counts the number of occurrences of   */
/* card type (ace, two, three, ..., queen, king)   */
/* at position 103 in a stack of cards ( 4 decks   */
/* of 52 cards) for 26,000 shuffles. Ideally the   */
/* number of cards observed of the same type at    */
/* position 103 (or any other position) would be   */
/* 26000 / 13 = 2000 assuming the random number    */
/* generator provides a uniform distribution.      */
/***************************************************/

/* timing stuff ripped out */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************/
/* Parameters for the mother of random  */
/* number generation programs: Mother() */
/****************************************/
#define m16Long   65536L        /* 2^16 */
#define m16Mask   0xFFFF        /* mask for lower 16 bits */
#define m15Mask   0x7FFF        /* mask for lower 15 bits */
#define m31Mask   0x7FFFFFFF    /* mask for 31 bits */
#define m32Double 4294967295.0  /* 2^32-1 */

static short mother1[10];
static short mother2[10];
static short mStart=1;

#define NDecks    4
int card[52*NDecks];
int suit[14];

int main(int argc, char* argv[])
{
 int  i, j, k, m, n;   
 int  ICard, NCards, NShuf, shuffle();
 
 double x, y;

 char cname[13][10]  = { "ACE", "TWO", "THREE",
			 "FOUR", "FIVE", "SIX",
			 "SEVEN", "EIGHT", "NINE",
			 "TEN", "JACK", "QUEEN",
			 "KING"
			 };

 /*####################*/
 /*#  Initialization  #*/
 /*####################*/
 NCards = 52 * NDecks;
 NShuf  = 26000;
 ICard  = 103;

 if (argc > 1)
  NShuf = atoi(argv[1]);

 for (i = 0; i < 14; i++)
    {
     suit[i] = 0;
    }

 printf("SHUFFLE: Test of Random Number Generator.\n");
 printf("Counts number of occurrences of card type at position\n");
 printf("%d in card deck for %d shuffles. Ideal would be\n",ICard,NShuf);
 printf("%d occurrences for each type of card.\n", NShuf/13);
 printf(" Card   Number    Percent\n");
 printf(" value  observed  from ideal\n");
 
 /********************************/
 /* Preset the NDecks of cards.  */
 /*  1 is Ace                    */
 /* 11 is Jack                   */
 /* 12 is Queen                  */
 /* 13 is King                   */
 /* We ignore the suit...        */
 /********************************/

 for (k = 0; k < NDecks; k++)
    {
     n = 52 * k;
     for (j = 0; j < 4; j++)
	  {
	   m = 13 * j + n;
	   for (i = 1; i < 14; i++)
		{
		 card[i + m - 1] = i;
		}
	  }
    }

 /***********************************/
 /* Pre-shuffle the NDecks of cards */
 /***********************************/
 shuffle();

 for (i = 1; i <= NShuf; i++)
    {
     shuffle();
     j = card[ICard];
     suit[j] = suit[j] + 1;
    }
 
 j = 0;
 y = 13.0 / (double)NShuf;
 for (i = 1; i < 14; i++)
    {
     k = suit[i];
     x = 100.0 * ((double)k * y - 1.0);
     printf("%5s   %5d    %5.1f\n",cname[i-1],k,x);
     j = j + k;
    }

 printf("Total Cards = %d\n",j);

 return(0);
}

int shuffle()
{
 int i, j, k, l, m, n;
 unsigned long kr = 3141592;
 double r, scale, Mother();
 
 /**************************************/
 /*  If you want to test rand() you    */
 /*  will need to set the value of     */
 /*  scale as follows:                 */
 /*  scale  = 1.0 / (double)RAND_MAX;  */
 /*                                    */
 /*  For Mother() however scale = 1.0; */
 /**************************************/
 scale = 1.0;

 j = 52 * NDecks;
 r = (double)j * scale;
 k = 10 * j;
 for (i = 1; i <= k; i++)
    {
     m = (int)(r * Mother(&kr));
     n = (int)(r * Mother(&kr));
     /****************************/
     /*  m = (int)(r * rand());  */
     /*  n = (int)(r * rand());  */
     /****************************/
     l = card[n];
     card[n] = card[m];
     card[m] = l;
    }

 return 0;
}

/* Mother *************************************************************
|  George Marsaglia's The mother of all random number generators
|   producing uniformly distributed pseudo random 32 bit values
|   with period about 2^250.
|  The text of Marsaglia's posting is provided in the file mother.txt
|
|  The arrays mother1 and mother2 store carry values in their
|   first element, and random 16 bit numbers in elements 1 to 8.
|   These random numbers are moved to elements 2 to 9 and a new
|   carry and number are generated and placed in elements 0 and 1.
|  The arrays mother1 and mother2 are filled with random 16 bit
|   values on first call of Mother by another generator.  mStart
|   is the switch.
|
|  Returns:
|   A 32 bit random number is obtained by combining the output of the
|   two generators and returned in *pSeed.  It is also scaled by
|   2^32-1 and returned as a double between 0 and 1
|
|  SEED:
|   The inital value of *pSeed may be any long value
|
|   Bob Wheeler 8/8/94
*/

double Mother(unsigned long *pSeed)
{
 unsigned long  number, number1, number2;
 short n, *p;
 unsigned short sNumber;

 /* Initialize motheri with 9 random values the first time */
 if (mStart) 
   {
    sNumber= *pSeed&m16Mask;   /* The low 16 bits */
    number = *pSeed&m31Mask;   /* Only want 31 bits */

    p = mother1;
    for (n=18;n--;)
	 {
	  number=30903*sNumber+(number>>16);   /* One line multiply-with-cary */
	  *p++=sNumber=number&m16Mask;
	  if (n==9) p = mother2;
	 }
    
    /* make cary 15 bits */
    mother1[0]&=m15Mask;
    mother2[0]&=m15Mask;
    mStart=0;
   }

 /* Move elements 1 to 8 to 2 to 9 */
 memmove(mother1+2,mother1+1,8*sizeof(short));
 memmove(mother2+2,mother2+1,8*sizeof(short));

 /* Put the carry values in numberi */
 number1=mother1[0];
 number2=mother2[0];

 /* Form the linear combinations */
 number1+=1941*mother1[2]+1860*mother1[3]+1812*mother1[4]+
 1776*mother1[5]+1492*mother1[6]+1215*mother1[7]+
 1066*mother1[8]+12013*mother1[9];
	
 number2+=1111*mother2[2]+2222*mother2[3]+3333*mother2[4]+
 4444*mother2[5]+5555*mother2[6]+6666*mother2[7]+
 7777*mother2[8]+9272*mother2[9];

 /* Save the high bits of numberi as the new carry */
 mother1[0]=number1/m16Long;
 mother2[0]=number2/m16Long;
		
 /* Put the low bits of numberi into motheri[1] */
 mother1[1]=m16Mask&number1;
 mother2[1]=m16Mask&number2;

 /* Combine the two 16 bit random numbers into one 32 bit */
 *pSeed=(((long)mother1[1])<<16)+(long)mother2[1];

 /* Return a double value between 0 and 1 */
 return ((double)*pSeed)/m32Double;
}

/* ---------- End of shuffle.c, Say goodnight Carrie ---------- */
