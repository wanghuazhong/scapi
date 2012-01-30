/*
 *   Proposed Digital Signature Standard (DSS)
 *
 *   Elliptic Curve variation GF(2^m) - See Dr. Dobbs Journal, April 1997
 *
 *   This program asks for the name of a <file>, computes its message digest,
 *   signs it, and outputs the signature to a file <file>.ecs. It is assumed 
 *   that curve parameters are available from a file common2.ecs, as well as 
 *   the private key of the signer previously generated by the ecsgen2 program
 *
 *   The curve is y^2+xy = x^3+Ax^2+B over GF(2^m) using a trinomial or 
 *   pentanomial basis (t^m+t^a+1 or t^m+t^a+t^b+t^c+1), These parameters
 *   can be generated using the findbase.cpp example program, or taken from tables
 *   provided, for example in IEEE-P1363 Annex A 
 *
 *   The file common2.ecs is presumed to exist and contain 
 *   {m,A,B,q,x,y,a,b,c} where A and B are parameters of the equation 
 *   above, (x,y) is an initial point on the curve, {m,a,b,c} are the field 
 *   parameters, (b is zero for a trinomial) and q is the order of the 
 *   (x,y) point, itself a large prime. The number of points on the curve is 
 *   cf.q where cf is the "co-factor", normally 2 or 4.
 *  
 *   This program is written for static mode.
 *   For a 163-bit modulus p, MR_STATIC could be defined as 6 in mirdef.h
 *   for a 32-bit processor, or 11 for a 16-bit processor (11*16 > 163).
 *   The system parameters can be found in the file common2.ecs
 *   Assumes MR_GENERIC_MT is defined in mirdef.h
 *
 *   Copyright (c) 2000-2007 Shamus Software Ltd.
 */

#include <stdio.h>
#include "miracl.h"
#include <stdlib.h>
#include <string.h>

void strip(char *name)
{ /* strip off filename extension */
    int i;
    for (i=0;name[i]!='\0';i++)
    {
        if (name[i]!='.') continue;
        name[i]='\0';
        break;
    }
}

static void hashing(miracl *mip,FILE *fp,big hash)
{ /* compute hash function */
    char h[20];
    int i,ch;
    sha sh;
    shs_init(&sh);
    while ((ch=fgetc(fp))!=EOF) shs_process(&sh,ch);
    shs_hash(&sh,h);
    bytes_to_big(mip,20,h,hash);
}

int main()
{
    FILE *fp;
    int m,a,b,c;
    char ifname[50],ofname[50];
    big a2,a6,q,x,y,d,r,s,k,hash;
    epoint *g;
    long seed;
    miracl instance;
    miracl *mip=&instance;
    char mem[MR_BIG_RESERVE(10)];           /* reserve space on the stack for 10 bigs */
    char mem1[MR_ECP_RESERVE(1)];           /* and one elliptic curve points         */
    memset(mem,0,MR_BIG_RESERVE(10));
    memset(mem1,0,MR_ECP_RESERVE(1));

/* get public data */
    fp=fopen("common2.ecs","rt");
    if (fp==NULL)
    {
        printf("file common2.ecs does not exist\n");
        return 0;
    }
    fscanf(fp,"%d\n",&m);

    mip=mirsys(mip,MR_ROUNDUP(abs(m),4),16);
    a2=mirvar_mem(mip,mem,0);
    a6=mirvar_mem(mip,mem,1);
    q=mirvar_mem(mip,mem,2);
    x=mirvar_mem(mip,mem,3);
    y=mirvar_mem(mip,mem,4);
    d=mirvar_mem(mip,mem,5);
    r=mirvar_mem(mip,mem,6);
    s=mirvar_mem(mip,mem,7);
    k=mirvar_mem(mip,mem,8);
    hash=mirvar_mem(mip,mem,9);

    innum(mip,a2,fp);     /* curve parameters */
    innum(mip,a6,fp);     /* curve parameters */
    innum(mip,q,fp);     /* order of (x,y) */
    innum(mip,x,fp);     /* (x,y) point on curve of order q */
    innum(mip,y,fp);
 
    fscanf(fp,"%d\n",&a);
    fscanf(fp,"%d\n",&b);
    fscanf(fp,"%d\n",&c);
    fclose(fp);

/* randomise */
    printf("Enter 9 digit random number seed  = ");
    scanf("%ld",&seed);
    getchar();
    irand(mip,seed);

    ecurve2_init(mip,m,a,b,c,a2,a6,FALSE,MR_PROJECTIVE);  /* initialise curve */

    g=epoint_init_mem(mip,mem1,0);
    epoint2_set(mip,x,y,0,g); /* set point of order q */

/* calculate r - this can be done offline, 
   and hence amortized to almost nothing   */
    bigrand(mip,q,k);
    ecurve2_mult(mip,k,g,g);      /* see ebrick2.c for method to speed this up */
    epoint2_get(mip,g,r,r);
    divide(mip,r,q,q);

/* get private key of signer */
    fp=fopen("private.ecs","rt");
    if (fp==NULL)
    {
        printf("file private.ecs does not exist\n");
        return 0;
    }
    innum(mip,d,fp);
    fclose(fp);

/* calculate message digest */
    printf("file to be signed = ");
    gets(ifname);
    strcpy(ofname,ifname);
    strip(ofname);
    strcat(ofname,".ecs");
    if ((fp=fopen(ifname,"rb"))==NULL)
    {
        printf("Unable to open file %s\n",ifname);
        return 0;
    }
    hashing(mip,fp,hash);
    fclose(fp);

/* calculate s */
    xgcd(mip,k,q,k,k,k);

    mad(mip,d,r,hash,q,q,s);
    mad(mip,s,k,k,q,q,s);
    fp=fopen(ofname,"wt");
    otnum(mip,r,fp);
    otnum(mip,s,fp);
    fclose(fp);
/* clear all memory used */
    memset(mem,0,MR_BIG_RESERVE(10));
    memset(mem1,0,MR_ECP_RESERVE(1));

    return 0;
}

