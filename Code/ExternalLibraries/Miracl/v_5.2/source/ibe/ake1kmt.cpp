/*
 *    No matter where you got this code from, be aware that MIRACL is NOT 
 *    free software. For commercial use a license is required.
 *	  See www.shamus.ie
 *
   Scott's AKE Client/Server testbed

   See http://www.compapp.dcu.ie/research/CA_Working_Papers/wp02.html#0202

   Compile as 
   cl /O2 /GX ake1kmt.cpp big.cpp zzn.cpp ecn.cpp miracl.lib

   Requires file xk1.ecs which contains details of non-supersingular 
   elliptic curve, with security multiplier k=1. The prime p is 1024 bits

   xk1.ecs is generated by make_k1.cpp
*/

#include <iostream>
#include <fstream>
#include "ecn.h"
#include "zzn.h"
#include <ctime>

using namespace std;

#ifdef MR_COUNT_OPS
extern "C"
{
    int fpc=0;
    int fpa=0;
    int fpx=0;
}
#endif

//#define ENDO

Miracl precision(32,0); 

// Using SHA-512 as basic hash algorithm

#define HASH_LEN 64

//
// Tate Pairing Code
//
// Extract ECn point in internal ZZn format
//

void extract(ECn& A,ZZn& x,ZZn& y)
{ 
    x=(A.get_point())->X;
    y=(A.get_point())->Y;
}

void extract(ECn& A,ZZn& x,ZZn& y,ZZn& z)
{ 
    big t;
    x=(A.get_point())->X;
    y=(A.get_point())->Y;
    t=(A.get_point())->Z;
    if (A.get_status()!=MR_EPOINT_GENERAL) z=(ZZn)1;
    else                                   z=t;
}

void insert(ECn& A,ZZn& x,ZZn& y,ZZn &z)
{
    copy(x.getzzn(),(A.get_point())->X);
    copy(y.getzzn(),(A.get_point())->Y);
    copy(z.getzzn(),(A.get_point())->Z); 
    A.get_point()->marker=MR_EPOINT_GENERAL;
}

//
// Add A=A+B  (or A=A+A) 
// Calculate numerators and denominators
// Line from A to destination B. Let A=(x,y)
// Now evaluate at Q -> return (Qy-y)-slope.(Qx-x)
// Also evaluate vertical line through destination
//

void g(ECn& A,ECn& B,ZZn& Qx,ZZn& Qy,ZZn& bQx,ZZn& num,ZZn& denom,ZZn& num2,ZZn& denom2,BOOL last)
{
    ZZn lam,x,y,z,x2,y2,z2,t1,t2,t3;
    ZZn zz,zzz;

// Evaluate line from A - lam is line slope

    extract(A,x,y,z);
    zz=z*z; zzz=zz*z; 

    if (&A==&B)
    {  // double
        t1=x*x; lam=t1+t1+t1; t2=y*y;
        t3=x*t2; t3+=t3; t3+=t3;
        x2=lam*lam-(t3+t3);
        z2=z*y; z2+=z2;
        t1=t2+t2; t2=t1*t1; t2+=t2;
        t3-=x2; y2=lam*t3-t2;
    }
    else
    { // add
        ZZn xs,ys;
        extract(B,xs,ys);

        t1=zz*xs;
        lam=zzz*ys; t1-=x; lam-=y; 
        z2=z*t1; t3=t1*t1; t2=t3*t1;
        t3*=x; t1=t3+t3; x2=lam*lam-t1-t2; t2*=y; 
        t3-=x2; y2=lam*t3-t2;   
    }

    t1=z2*z2;
    lam*=z; lam*=z2;
    t2=zzz*Qy;

    num=t1*(t2-y)-lam*(zz*Qx-x);
    denom=zzz*(t1*Qx-x2);
    if (!last)
    {
        num2=t1*(t2+y)+lam*(zz*bQx-x);
        denom2=zzz*(t1*bQx-x2);
    }
    else
    {
        num2=-zz*bQx+x;
        denom2=zz;
    }

    insert(A,x2,y2,z2);
    return;
}


void g(ECn& A,ECn& B,ZZn& Qx,ZZn& Qy,ZZn& num,ZZn& denom,BOOL last)
{
    ZZn lam,x,y,z,x2,y2,z2,t1,t2,t3;
    ZZn zz,zzz;

// Evaluate line from A - lam is line slope

    extract(A,x,y,z);
    zz=z*z; zzz=zz*z; 

    if (&A==&B)
    {  // double
        t1=x*x; lam=t1+t1+t1; t2=y*y;
        t3=x*t2; t3+=t3; t3+=t3;
        x2=lam*lam-(t3+t3);
        z2=z*y; z2+=z2;
        t1=t2+t2; t2=t1*t1; t2+=t2;
        t3-=x2; y2=lam*t3-t2;
    }
    else
    { // add
        ZZn xs,ys;
        extract(B,xs,ys);

        t1=zz*xs;
        lam=zzz*ys; t1-=x; lam-=y; 
        z2=z*t1; t3=t1*t1; t2=t3*t1;
        t3*=x; t1=t3+t3; x2=lam*lam-t1-t2; t2*=y; 
        t3-=x2; y2=lam*t3-t2;   
    }

    t1=z2*z2;
    lam*=z; lam*=z2;
    t2=zzz*Qy;

	if (!last)
	{
		num=t1*(t2-y)-lam*(zz*Qx-x);
		denom=zzz*(t1*Qx-x2);
	}
	else
	{
		num=-zz*Qx+x;
		denom=zz;
	}

    insert(A,x2,y2,z2);
    return;
}


//
// Tate Pairing - Note no denominator elimination possible
//
// P is a point of order q. Q(x,y) is a point of order m.q. 
// Note that P and Q are points on the curve over Fp
//

BOOL fast_tate_pairing(ECn& P,ZZn& Qx,ZZn& Qy,ZZn& bQx,Big& q,Big& cofactor,ZZn& res)
{ 
    int i,j,nb;
    ECn A,W;
    ZZn ha,had,hab,ha2,had2;
    ZZn n,d,n2,d2;

    ha=had=ha2=had2=1;  

#ifdef MR_COUNT_OPS
fpc=fpa=fpx=0;
#endif
    
    normalise(P);

    A=P;
#ifndef ENDO
// Here I am not using the endomorphism...
	nb=bits(q);
	for (i=nb-2;i>=1;i--)
	{
	   ha*=ha;
	   had*=had;
	   g(A,A,Qx,Qy,n,d,FALSE);
	   ha*=n;
	   had*=d;
	   if (bit(q,i)==1)
	   {
		   g(A,P,Qx,Qy,n,d,FALSE);
		   ha*=n;
		   had*=d;
	   }
	}
	ha*=ha;
	had*=had;
	g(A,A,Qx,Qy,n,d,FALSE);
	ha*=n;
	had*=d;

	g(A,P,Qx,Qy,n,d,TRUE);
	ha*=n;
	had*=d;   
    
	ha/=had;

#else

 //   lambda=pow((Big)2,80)+pow((Big)2,16);
 //   Use endomorphism for half length loop..

    for (i=0;i<64;i++)
    {
        ha*=ha; ha2*=ha2;
        had*=had; had2*=had2;
        g(A,A,Qx,Qy,bQx,n,d,n2,d2,FALSE);
   
        ha*=n; ha2*=n2;
        had*=d; had2*=d2;
    }

    g(A,P,Qx,Qy,bQx,n,d,n2,d2,FALSE);

    ha*=n; ha2*=n2;
    had*=d; had2*=d2;

    for (i=0;i<16;i++)
    {
        ha*=ha; ha2*=ha2;
        had*=had; had2*=had2;
        g(A,A,Qx,Qy,bQx,n,d,n2,d2,FALSE);

        ha*=n; ha2*=n2;
        had*=d; had2*=d2;
    }

    g(A,P,Qx,Qy,bQx,n,d,n2,d2,TRUE);
    ha*=n; had*=d;
    ha2*=n2; had2*=d2;

    ha/=had;
    ha2/=had2;

    hab=ha;
    for (i=0;i<64;i++) ha*=ha;
    ha*=hab;
    for (i=0;i<16;i++) ha*=ha;
    ha*=ha2;

#endif
    res=ha;
    res=res*res*res;       // wipes out multiples of zeta
    res=pow(res,cofactor); // since p=1 mod 4, -1 gets wiped out as well
    res=pow(res,cofactor);
    res=pow(res,q);

#ifdef MR_COUNT_OPS
cout << "Tate fpc= " << fpc << endl;
cout << "Tate fpa= " << fpa << endl;
cout << "Tate fpx= " << fpx << endl;
fpa=fpc=fpx=0;
#endif

    if (res==(ZZn)1) return FALSE;
    return TRUE;            
}

//
// Pairing ecap(.) function
//

BOOL ecap(ECn& P,ECn& Q,Big& order,ZZn& zeta,Big& cofactor,ZZn& res)
{
    BOOL Ok;
    Big Qx,Qy;
    ZZn bQx;

    Q.get(Qx,Qy);
    bQx=zeta*zeta*(ZZn)Qx;
    Ok=fast_tate_pairing(P,(ZZn)Qx,(ZZn)Qy,bQx,order,cofactor,res);

    if (Ok) return TRUE;
    return FALSE;
}

//
// Hash functions
// 

Big H1(char *string)
{ // Hash a zero-terminated string to a number < modulus
    Big h,p;
    char s[HASH_LEN];
    int i,j; 
    sha512 sh;

    shs512_init(&sh);

    for (i=0;;i++)
    {
        if (string[i]==0) break;
        shs512_process(&sh,string[i]);
    }
    shs512_hash(&sh,s);
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

Big H2(ZZn x)
{ // Hash an Fp to a big number
    sha sh;
    Big a,b,h,p;
    char s[20];
    int i,j,m;

    shs_init(&sh);
    a=x;
    while (a>0)
    {
        m=a%256;
        shs_process(&sh,m);
        a/=256;
    }
    shs_hash(&sh,s);
    h=from_binary(20,s);
    return h;
}

// Hash and map an Identity to a curve point E_(Fp)

ECn hash_and_map(char *ID,Big cof)
{
    ECn Q;
    Big x0=H1(ID);

    while (!Q.set(x0)) x0+=1;
    Q*=cof;
    return Q;
}

ECn hash(char *ID)
{
    ECn Q;
    Big x0=H1(ID);

    while (!Q.set(x0)) x0+=1;
 
    return Q;
}

ECn map_f(ECn& P,ZZn& zeta)
{
    ECn R;
    Big x,y;
    P.get(x,y);
    ZZn X=x;
    X=zeta*X; x=Big(X);
    if (!R.set(x,y)) cout << "something wrong in map_f" << endl;   
    return R;
}

ECn pi1(ECn P,ZZn& zeta,Big lambda)
{
    ECn T,R;
    T=lambda*P;
    R=map_f(P,zeta);
    R-=T;
    return R;
}

ECn pi2(ECn P,ZZn& zeta,Big lambda)
{
    ECn T,R;
    T=lambda*P;
    T=lambda*T;
    R=map_f(P,zeta);
    R-=T;
    return R;
}

int main()
{
    ifstream common("xk1.ecs");      // elliptic curve parameters
    miracl* mip=&precision;
    ECn Alice,Bob,sA,sB;
    ECn B2,Server,sS;
    ZZn X,Y,res,sp,ap,bp,c,third,zeta;
    Big r,a,b,s,ss,p,q,x,y,B,cf,t,lambda,cofactor;
    int i,bts,A;
    time_t seed;

    common >> bts;
    mip->IOBASE=16;
    common >> p;
    common >> A;
    common >> B;

//    q=pow((Big)2,159)+pow((Big)2,17)+1;

    lambda=pow((Big)2,80)+pow((Big)2,16);
    q=lambda*lambda+lambda+1;

    cofactor=sqrt((p-1)/(3*q*q));
    cf=3*cofactor;
    cout << "cofactor in bits= " << bits(cf) << endl;
//    cout << "cf= " << cf << endl;

    i=1;
    do
    {
        zeta=(ZZn)pow((Big)++i,(p-1)/3,p);
    } while (zeta==1);
//    cout << "zeta= " << zeta << endl;
//    zeta*=zeta;   // may be needed to get right root...

    i=2;
    do
    {
        lambda=pow((Big)++i,(q-1)/3,q);
    } while (lambda==1);
//    cout << "lambda= " << lambda << endl;

    time(&seed);
    irand((long)seed);

    ecurve(A,B,p,MR_PROJECTIVE);

    mip->IOBASE=16;

// hash Identities to curve point

    ss=rand(q);    // TA's super-secret 

    cout << "Mapping Server ID to point" << endl;

    Server=hash_and_map("Server",cf);
//    Server=hash("Server"); //Server*=3;
    Server=pi2(Server,zeta,lambda); // Place Server into distinguised group 2
//    cout << "Server= " << Server << endl;

    cout << "Mapping Alice & Bob ID's to points" << endl;
    
    Alice=hash_and_map("Alice",cf);
    Alice=pi1(Alice,zeta,lambda);      // got to put Alice and Bob into David's "distinguished" group								 
										 // so that endomorphism applies
    Bob=  hash_and_map("Robert",cf);
    Bob=pi1(Bob,zeta,lambda); 

//	ecap(Alice,map_f(Bob,zeta),q,zeta,cofactor,res);
//	cout << "e(Alice,phi(Bob)) = " << res << endl;
//	ecap(Bob,map_f(Alice,zeta),q,zeta,cofactor,res);
//	cout << "e(Bob,phi(Alice)) = " << res << endl;

//	exit(0);

// Calculating e(P,P) != 1 ??

    third=inverse((ZZn)3,q);               // 1/3 mod q
    ecap(Alice,3*Alice,q,zeta,cofactor,res);             // Note the 3
    cout << "e(Alice,Alice)=  " << pow(res,third) << endl;
//exit(0);
    ecap(Alice,Bob,q,zeta,cofactor,res);
    cout << "e(Alice,Bob)= " << res << endl;
    ecap(Bob,Alice,q,zeta,cofactor,res);
    cout << "e(Bob,Alice)= " << res << endl;

    ecap(Alice,Server,q,zeta,cofactor,res);
    cout << "e(Alice,Server)= " << res << endl;

// The pairing is symmetric! 
    zeta*=zeta; // (its the "other" cube root) needed with ENDO
    ecap(Server,Alice,q,zeta,cofactor,res);
    cout << "e(Server,Alice)= " << res << endl;
    zeta*=zeta;

    cout << "Alice, Bob and the Server visit Trusted Authority" << endl;

    sS=ss*Server; 
    sA=ss*Alice;
    sB=ss*Bob;

    cout << "Alice and Server Key exchange" << endl;

    a=rand(q);   // Alice's random number
    s=rand(q);   // Server's random number
//#ifndef MR_COUNT_OPS
//for (i=0;i<1000;i++)
//#endif
    if (!ecap(sA,Server,q,zeta,cofactor,res)) cout << "Trouble" << endl;
    ap=pow(res,a);

    if (!ecap(Alice,sS,q,zeta,cofactor,res)) cout << "Trouble" << endl;
    sp=pow(res,s);

    cout << "Alice  Key= " << H2(pow(sp,a)) << endl;
    cout << "Server Key= " << H2(pow(ap,s)) << endl;

    cout << "Bob and Server Key exchange" << endl;

    b=rand(q);   // Bob's random number
    s=rand(q);   // Server's random number

    if (!ecap(sB,Server,q,zeta,cofactor,res)) cout << "Trouble" << endl;
    bp=pow(res,b);

    if (!ecap(Bob,sS,q,zeta,cofactor,res)) cout << "Trouble" << endl;
    sp=pow(res,s);

    cout << "Bob's  Key= " << H2(pow(sp,b)) << endl;
    cout << "Server Key= " << H2(pow(bp,s)) << endl;

    cout << "Alice and Bob's attempted Key exchange" << endl;

    ecap(sB,Alice,q,zeta,cofactor,res);
    ap=pow(res,a);

    ecap(Bob,sA,q,zeta,cofactor,res);
    bp=pow(res,b);

    if (ap==1) cout << "Bob's   Key = 1" << endl;
    else cout << "Bob's   Key= " << H2(pow(ap,b)) << endl;
    if (bp==1) cout << "Alice's Key = 1" << endl;
    else cout << "Alice's Key= " << H2(pow(bp,a)) << endl;

    return 0;
}
