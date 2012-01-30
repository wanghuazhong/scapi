/*
 *    No matter where you got this code from, be aware that MIRACL is NOT 
 *    free software. For commercial use a license is required.
 *	  See www.shamus.ie
 *
   Scott's AKE Client/Server testbed

   See http://eprint.iacr.org/2002/164

   On Windows compile as 
   cl /O2 /GX /DZZNS=10 ake24blsa.cpp zzn24.cpp zzn8.cpp zzn4.cpp zzn2.cpp zzn.cpp ecn.cpp ecn4.cpp big.cpp miracl.lib

   for 64-bit computer. Change to /DZZNS=20 for 32-bit computer

   On Linux compile as
   g++ -O2 -DZZNS=10 ake24blsa.cpp zzn24.cpp zzn8.cpp zzn4.cpp zzn2.cpp zzn.cpp ecn.cpp ecn4.cpp big.cpp miracl.a -o ake24blsa

   Barreto-Lynn-Scott k=24 Curve - ate pairing

   The BLS curve generated is generated from an x parameter
   This version implements the ate pairing (which is optimal in this case)

   See bls24.cpp for a program to generate suitable bls24 curves

   Modified to prevent sub-group confinement attack
*/

#include <iostream>
#include <fstream>
#include <string.h>
#include "ecn.h"
#include <ctime>
#include "ecn4.h"
#include "zzn24.h"

using namespace std;

#ifdef MR_COUNT_OPS
extern "C"
{
    int fpc=0;
    int fpa=0;
    int fpx=0;
	int fpm2=0;
	int fpi2=0;
	int fpaq=0;
	int fpsq=0;
	int fpmq=0;
}
#endif

#if MIRACL==64
Miracl precision(10,0); 
#else
Miracl precision(20,0);
#endif

// Using SHA-256 as basic hash algorithm

#define HASH_LEN 32

ZZn24 Frobenius(ZZn24 P, ZZn2 &X, int k)
{
  ZZn24 Q=P;
  for (int i=0; i<k; i++)
    Q.powq(X);
  return Q;
}

// Suitable for p=7 mod 12

void set_frobenius_constant(ZZn2 &X)
{
    Big p=get_modulus();
	X.set((Big)1,(Big)1);  // p=3 mod 8
	X=pow(X,(p-7)/12); 
}

//
// Line from A to destination C. Let A=(x,y)
// Line Y-slope.X-c=0, through A, so intercept c=y-slope.x
// Line Y-slope.X-y+slope.x = (Y-y)-slope.(X-x) = 0
// Now evaluate at Q -> return (Qy-y)-slope.(Qx-x)
//

ZZn24 line(ECn4& A,ECn4& C,ZZn4& slope,ZZn& Qx,ZZn& Qy)
{
     ZZn24 w;
     ZZn8 nn,dd;
     ZZn4 X,Y;

     A.get(X,Y);
	
	 nn.set((ZZn4)-Qy,Y-slope*X);
	 dd.set(slope*Qx);
	 w.set(nn,dd);

     return w;
}

//
// Add A=A+B  (or A=A+A) 
// Return line function value
//

ZZn24 g(ECn4& A,ECn4& B,ZZn& Qx,ZZn& Qy)
{
    ZZn4 lam;
    ZZn24 r;
    ECn4 P=A;

// Evaluate line from A
    A.add(B,lam);
    if (A.iszero())   return (ZZn24)1; 
    r=line(P,A,lam,Qx,Qy);

    return r;
}

//
// This calculates p.A = (X^p,Y^p) quickly using Frobenius
// 1. Extract A(x,y) from twisted curve to point on curve over full extension, as X=i^2.x and Y=i^3.y
// where i=NR^(1/k)
// 2. Using Frobenius calculate (X^p,Y^p)
// 3. map back to twisted curve
// Here we simplify things by doing whole calculation on the twisted curve
//
// Note we have to be careful as in detail it depends on w where p=w mod k
// Its simplest if w=1.
//

ECn4 psi(ECn4 &A,ZZn2 &F,int n)
{ 
	int i;
	ECn4 R;
	ZZn4 X,Y;
	ZZn2 FF,W;
// Fast multiplication of A by q^n
    A.get(X,Y);
	FF=F*F;
	W=txx(txx(txx(FF*FF*FF)));

	for (i=0;i<n;i++)
	{ // assumes p=7 mod 12
		X.powq(W); X=tx(tx(FF*X));
		Y.powq(W); Y=tx(tx(tx(FF*F*Y)));
	}
	
    R.set(X,Y);
	return R;
}

// Automatically generated by Luis Dominguez

ZZn24 HardExpo(ZZn24 &f3x0, ZZn2 &X, Big &x){
//vector=[ 1, 2, 3 ]
  ZZn24 r;
  ZZn24 xA;
  ZZn24 xB;
  ZZn24 t0;
  ZZn24 t1;
  ZZn24 f3x1;
  ZZn24 f3x2;
  ZZn24 f3x3;
  ZZn24 f3x4;
  ZZn24 f3x5;
  ZZn24 f3x6;
  ZZn24 f3x7;
  ZZn24 f3x8;
  ZZn24 f3x9;

  f3x1=pow(f3x0,x);
  f3x2=pow(f3x1,x);
  f3x3=pow(f3x2,x);
  f3x4=pow(f3x3,x);
  f3x5=pow(f3x4,x);
  f3x6=pow(f3x5,x);
  f3x7=pow(f3x6,x);
  f3x8=pow(f3x7,x);
  f3x9=pow(f3x8,x);

  xA=f3x4*inverse(f3x8)*Frobenius(f3x3,X,1)*Frobenius(inverse(f3x7),X,1)*Frobenius(f3x2,X,2)*Frobenius(inverse(f3x6),X,2)*Frobenius(f3x1,X,3)*Frobenius(inverse(f3x5),X,3)*Frobenius(inverse(f3x4),X,4)*Frobenius(inverse(f3x3),X,5)*Frobenius(inverse(f3x2),X,6)*Frobenius(inverse(f3x1),X,7);
  xB=f3x0;
  t0=xA*xB;
  xA=inverse(f3x3)*inverse(f3x5)*f3x7*f3x9*Frobenius(inverse(f3x2),X,1)*Frobenius(inverse(f3x4),X,1)*Frobenius(f3x6,X,1)*Frobenius(f3x8,X,1)*Frobenius(inverse(f3x1),X,2)*Frobenius(inverse(f3x3),X,2)*Frobenius(f3x5,X,2)*Frobenius(f3x7,X,2)*Frobenius(inverse(f3x0),X,3)*Frobenius(inverse(f3x2),X,3)*Frobenius(f3x4,X,3)*Frobenius(f3x6,X,3)*Frobenius(f3x3,X,4)*Frobenius(f3x5,X,4)*Frobenius(f3x2,X,5)*Frobenius(f3x4,X,5)*Frobenius(f3x1,X,6)*Frobenius(f3x3,X,6)*Frobenius(f3x0,X,7)*Frobenius(f3x2,X,7);
  xB=f3x0;
  t1=xA*xB;
  t0=t0*t0;
  t0=t0*t1;

r=t0;
return r;

}

void SoftExpo(ZZn24 &f3, ZZn2 &X){
  ZZn24 t0;
  int i;

  t0=f3; f3.conj(); f3/=t0;
  f3.mark_as_regular();
  t0=f3; for (i=1;i<=4;i++)  f3.powq(X); f3*=t0;
  f3.mark_as_unitary();
}

//
// R-ate Pairing - note denominator elimination has been applied
//
// P is a point of order q. Q(x,y) is a point of order q. 
// Note that P is a point on the sextic twist of the curve over Fp^2, Q(x,y) is a point on the 
// curve over the base field Fp
//

BOOL fast_pairing(ECn4& P,ZZn& Qx,ZZn& Qy,Big &x,ZZn2 &X,ZZn24& r)
{ 
    ECn4 A;
	Big n;
	int i,nb;

#ifdef MR_COUNT_OPS
fpc=fpa=fpx=0;
#endif

    n=x;           // t-1
    A=P;           // remember A
 
    nb=bits(n);
    r=1;
	r.mark_as_miller();
//fpc=fpa=fpx=0;
    for (i=nb-2;i>=0;i--)
    {
        r*=r;   
        r*=g(A,A,Qx,Qy); 
 
        if (bit(n,i)) 
            r*=g(A,P,Qx,Qy);  
    }

    if (r.iszero()) return FALSE;
#ifdef MR_COUNT_OPS
cout << "Miller fpc= " << fpc << endl;
cout << "Miller fpa= " << fpa << endl;
cout << "Miller fpx= " << fpx << endl;
fpa=fpc=fpx=0;
#endif
	SoftExpo(r,X);
	r=HardExpo(r,X,x);
#ifdef MR_COUNT_OPS
cout << "FE fpc= " << fpc << endl;
cout << "FE fpa= " << fpa << endl;
cout << "FE fpx= " << fpx << endl;
fpa=fpc=fpx=0;
#endif
    return TRUE;
}

//
// ecap(.) function
//

BOOL ecap(ECn4& P,ECn& Q,Big& x,ZZn2 &X,ZZn24& r)
{
    BOOL Ok;
    Big xx,yy;
    ZZn Qx,Qy;

    Q.get(xx,yy); Qx=xx; Qy=yy;

    Ok=fast_pairing(P,Qx,Qy,x,X,r);

    if (Ok) return TRUE;
    return FALSE;
}

// Automatically generated by Luis Dominguez

ECn4 HashG2(ECn4& Qx0, Big& x, ZZn2& X){
//vector=[ 1, 2, 3, 4 ]
  ECn4 r;
  ECn4 xA;
  ECn4 xB;
  ECn4 xC;
  ECn4 t0;
  ECn4 t1;
  ECn4 Qx0_;
  ECn4 Qx1;
  ECn4 Qx1_;
  ECn4 Qx2;
  ECn4 Qx2_;
  ECn4 Qx3;
  ECn4 Qx3_;
  ECn4 Qx4;
  ECn4 Qx4_;
  ECn4 Qx5;
  ECn4 Qx5_;
  ECn4 Qx6;
  ECn4 Qx6_;
  ECn4 Qx7;
  ECn4 Qx7_;
  ECn4 Qx8;
  ECn4 Qx8_;

  Qx0_=-(Qx0);
  Qx1=x*Qx0;
  Qx1_=-(Qx1);
  Qx2=x*Qx1;
  Qx2_=-(Qx2);
  Qx3=x*Qx2;
  Qx3_=-(Qx3);
  Qx4=x*Qx3;
  Qx4_=-(Qx4);
  Qx5=x*Qx4;
  Qx5_=-(Qx5);
  Qx6=x*Qx5;
  Qx6_=-(Qx6);
  Qx7=x*Qx6;
  Qx7_=-(Qx7);
  Qx8=x*Qx7;
  Qx8_=-(Qx8);

  xA=Qx0;
  xC=Qx7;
  xA+=xC;
  xC=psi(Qx2,X,4);
  xA+=xC;
  xB=Qx0;
  xC=Qx7;
  xB+=xC;
  xC=psi(Qx2,X,4);
  xB+=xC;
  t0=xA+xB;
  xB=Qx2_;
  xC=Qx3_;
  xB+=xC;
  xC=Qx8_;
  xB+=xC;
  xC=psi(Qx2,X,1);
  xB+=xC;
  xC=psi(Qx3_,X,1);
  xB+=xC;
  xC=psi(Qx1,X,6);
  xB+=xC;
  t0=t0+xB;
  xB=Qx4;
  xC=Qx5_;
  xB+=xC;
  xC=psi(Qx0_,X,4);
  xB+=xC;
  xC=psi(Qx4_,X,4);
  xB+=xC;
  xC=psi(Qx0,X,5);
  xB+=xC;
  xC=psi(Qx1_,X,5);
  xB+=xC;
  xC=psi(Qx2_,X,5);
  xB+=xC;
  xC=psi(Qx3,X,5);
  xB+=xC;
  t0=t0+xB;
  xA=Qx1;
  xC=psi(Qx0_,X,1);
  xA+=xC;
  xC=psi(Qx1,X,1);
  xA+=xC;
  xC=psi(Qx4_,X,1);
  xA+=xC;
  xC=psi(Qx5,X,1);
  xA+=xC;
  xC=psi(Qx0,X,2);
  xA+=xC;
  xC=psi(Qx1_,X,2);
  xA+=xC;
  xC=psi(Qx4_,X,2);
  xA+=xC;
  xC=psi(Qx5,X,2);
  xA+=xC;
  xC=psi(Qx0,X,3);
  xA+=xC;
  xC=psi(Qx1_,X,3);
  xA+=xC;
  xC=psi(Qx4_,X,3);
  xA+=xC;
  xC=psi(Qx5,X,3);
  xA+=xC;
  xC=psi(Qx1,X,4);
  xA+=xC;
  xC=psi(Qx3,X,4);
  xA+=xC;
  xC=psi(Qx0_,X,6);
  xA+=xC;
  xC=psi(Qx2_,X,6);
  xA+=xC;
  xB=Qx4;
  xC=Qx5_;
  xB+=xC;
  xC=psi(Qx0_,X,4);
  xB+=xC;
  xC=psi(Qx4_,X,4);
  xB+=xC;
  xC=psi(Qx0,X,5);
  xB+=xC;
  xC=psi(Qx1_,X,5);
  xB+=xC;
  xC=psi(Qx2_,X,5);
  xB+=xC;
  xC=psi(Qx3,X,5);
  xB+=xC;
  t1=xA+xB;
  t0=t0+t0;
  t0=t0+t1;

r=t0;
return r;
}

//
// Hash functions
// 

Big H2(ZZn24 x)
{ // Compress and hash an Fp24 to a big number
    sha256 sh;
    ZZn8 u;
    ZZn4 h,l;
	ZZn2 t,b;
    Big a,hash,p;
	ZZn xx[8];
    char s[HASH_LEN];
    int i,j,m;

    shs256_init(&sh);
    x.get(u);  // compress to single ZZn4
    u.get(l,h);

	l.get(t,b);

	t.get(xx[0],xx[1]);
	b.get(xx[2],xx[3]);
    
	h.get(t,b);

	t.get(xx[4],xx[5]);
	b.get(xx[6],xx[7]);
 
    for (i=0;i<8;i++)
    {
        a=(Big)xx[i];
        while (a>0)
        {
            m=a%256;
            shs256_process(&sh,m);
            a/=256;
        }
    }
    shs256_hash(&sh,s);
    hash=from_binary(HASH_LEN,s);
    return hash;
}

Big H1(char *string)
{ // Hash a zero-terminated string to a number < modulus
    Big h,p;
    char s[HASH_LEN];
    int i,j; 
    sha256 sh;

    shs256_init(&sh);

    for (i=0;;i++)
    {
        if (string[i]==0) break;
        shs256_process(&sh,string[i]);
    }
    shs256_hash(&sh,s);
    p=get_modulus();
    h=1; j=0; i=1;
    forever
    {
        h*=256; 
        if (j==HASH_LEN)  {h+=i++; j=0;}
        else         h+=s[j++];
        if (h>=p) break;
    }
    h%=p;
    return h;
}

// Hash and map a Server Identity to a curve point E_(Fp4)

ECn4 hash_and_map4(char *ID)
{
    int i;
    ECn4 S;
    ZZn4 X;
	ZZn2 t;
 
    Big x0=H1(ID);
	
    forever
    {
        x0+=1;
		t.set((ZZn)0,(ZZn)x0);
        X.set(t,(ZZn2)0);
        if (!S.set(X)) continue;
        break;
    }
  
    return S;
}   

// Hash and Map a Client Identity to a curve point E_(Fp) of order q

ECn hash_and_map(char *ID,Big cf)
{
    ECn Q;
    Big x0=H1(ID);
    while (!Q.set(x0,x0)) x0+=1;
    Q*=cf;
    return Q;
}

void endomorph(ECn &A,ZZn &Beta)
{ // apply endomorphism P(x,y) = (Beta*x,y) where Beta is cube root of unity
  // Actually (Beta*x,-y) =  x^4.P
	ZZn x,y;
	x=(A.get_point())->X;
	y=(A.get_point())->Y;
	y=-y;
	x*=Beta;
	copy(getbig(x),(A.get_point())->X);
	copy(getbig(y),(A.get_point())->Y);
}

// Use GLV endomorphism idea for multiplication in G1.

ECn G1_mult(ECn &P,Big &e,Big &x,ZZn &Beta)
{
//	return e*P;
	int i;
	ECn Q;
	Big x4,u[2];
	x4=x*x; x4*=x4;
	u[0]=e%x4; u[1]=e/x4;

	Q=P;
	endomorph(Q,Beta);
	Q=mul(u[0],P,u[1],Q);
	
	return Q;
}

//.. for multiplication in G2

ECn4 G2_mult(ECn4 &P,Big e,Big &x,ZZn2 &X)
{
//	return e*P;
	int i;
	ECn4 Q[8];
	Big u[8];

	for (i=0;i<8;i++) {u[i]=e%x; e/=x;}

	Q[0]=P;
	for (i=1;i<8;i++)
		Q[i]=psi(Q[i-1],X,1);

// simple multi-addition

	return mul(8,Q,u);
}

//.. and for exponentiation in GT

ZZn24 GT_pow(ZZn24 &res,Big e,Big &x,ZZn2 &X)
{
//	return pow(res,e);
	int i,j;
	ZZn24 Y[8];
	Big u[8];

	for (i=0;i<8;i++) {u[i]=e%x; e/=x;}

	Y[0]=res;
	for (i=1;i<8;i++)
		{Y[i]=Y[i-1]; Y[i].powq(X);}

// simple multi-exponentiation
	return pow(8,Y,u);
}

// Fast group membership check for GT
// check if r is of order q
// Test r^q=r^{(p+1-t)/cf}= 1
// so test r^p=r^x and r^cf !=1
// exploit cf=(x-1)*(x-1)/3

BOOL member(ZZn24 &r,Big &x,ZZn2 &X)
{
	ZZn24 w=r;
	ZZn24 rx;
	if (r*conj(r)!=(ZZn24)1) return FALSE; // not unitary
	w.powq(X);
	rx=pow(r,x);
	if (w!=rx) return FALSE;
	if (r*pow(rx,x)==rx*rx) return FALSE;
	return TRUE;
}

int main()
{
    miracl* mip=&precision;
	ZZn2 X;
    ECn Alice,Bob,sA,sB;
    ECn4 Server,sS;
    ZZn24 sp,ap,bp,res;
    Big a,b,s,ss,p,q,x,y,B,cf,t,n;
	ZZn Beta;
    int i;
	time_t seed;

    mip->IOBASE=16;
	x="E000000000058400";  // low Hamming weight = 7
	t=1+x;
	p=(1+x+x*x-pow(x,4)+2*pow(x,5)-pow(x,6)+pow(x,8)-2*pow(x,9)+pow(x,10))/3;
	q=1-pow(x,4)+pow(x,8);
	
    n=p+1-t;
	cf=(x-1)*(x-1)/3; //=n/q

	cf=(x-1);  // Neat trick! Whole group is non-cyclic - just has (x-1)^2 as a factor
	           // So multiplication by x-1 is sufficient to create a point of order q

    ecurve((Big)0,(Big)6,p,MR_AFFINE);
	set_frobenius_constant(X);
	Beta=pow((ZZn)2,(p-1)/3);
	Beta*=Beta;   // right cube root of unity

    time(&seed);
    irand((long)seed);

	ss=rand(q);    // TA's super-secret 

    mip->TWIST=MR_SEXTIC_D;   // map Server to point on twisted curve E(Fp4)

    Server=hash_and_map4((char *)"Server");
	Server=HashG2(Server,x,X); // fast multiplication by co-factor

//  Should be point at infinity
//	cout << "psi^2(Server-t*psi(Server)+p*Server= " << psi(Server,X,2)-t*psi(Server,X,1)+p*Server << endl;
	
    Alice=hash_and_map((char *)"Alice",cf);
    Bob=  hash_and_map((char *)"Robert",cf); 

	cout << "Alice, Bob and the Server visit Trusted Authority" << endl; 

	sS=G2_mult(Server,ss,x,X);
	sA=G1_mult(Alice,ss,x,Beta);
    sB=G1_mult(Bob,ss,x,Beta);

    cout << "Alice and Server Key Exchange" << endl;

    a=rand(q);   // Alice's random number
    s=rand(q);   // Server's random number

    if (!ecap(Server,sA,x,X,res)) cout << "Trouble" << endl;
	if (!member(res,x,X))
    {
        cout << "Wrong group order - aborting" << endl;
        exit(0);
    }

	ap=GT_pow(res,a,x,X);  

    if (!ecap(sS,Alice,x,X,res)) cout << "Trouble" << endl;
    if (!member(res,x,X))
    {
        cout << "Wrong group order - aborting" << endl;
        exit(0);
    }

    sp=GT_pow(res,s,x,X);

    cout << "Alice  Key= " << H2(GT_pow(sp,a,x,X)) << endl;
    cout << "Server Key= " << H2(GT_pow(ap,s,x,X)) << endl;

    cout << "Bob and Server Key Exchange" << endl;

    b=rand(q);   // Bob's random number
    s=rand(q);   // Server's random number

    if (!ecap(Server,sB,x,X,res)) cout << "Trouble" << endl;
    if (!member(res,x,X))
    {
        cout << "Wrong group order - aborting" << endl;
        exit(0);
    }
 
	bp=GT_pow(res,b,x,X);

    if (!ecap(sS,Bob,x,X,res)) cout << "Trouble" << endl;
    if (!member(res,x,X))
    {
        cout << "Wrong group order - aborting" << endl;
        exit(0);
    }

	sp=GT_pow(res,s,x,X);

    cout << "Bob's  Key= " << H2(GT_pow(sp,b,x,X)) << endl;
    cout << "Server Key= " << H2(GT_pow(bp,s,x,X)) << endl;

	return 0;
}
