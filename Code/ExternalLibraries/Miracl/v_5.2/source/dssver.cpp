/*
 *   Digital Signature Algorithm (DSA)
 *
 *   See Communications ACM July 1992, Vol. 35 No. 7
 *   This new standard for digital signatures has been proposed by 
 *   the American National Institute of Standards and Technology (NIST)
 *   under advisement from the National Security Agency (NSA). 
 *
 *   This program verifies a the signature given to a <file> in
 *   <file>.dss generated by program dssign.c
 * 
 *   Requires: big.cpp
 *
 *   Copyright (c) 1988-1997 Shamus Software Ltd.
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include "big.h"

using namespace std;

Miracl precision(200,256);

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

static Big hash(ifstream &fp)
{ /* compute hash function */
    char ch,s[20];
    int i;
    Big h;
    sha sh;
    shs_init(&sh);
    forever 
    { /* read in bytes from message file */
        fp.get(ch);
        if (fp.eof()) break;
        shs_process(&sh,ch);
    }
    shs_hash(&sh,s);
    h=from_binary(20,s);
    return h;
}

int main()
{
    ifstream common("common.dss");    /* construct file I/O streams */
    ifstream public_key("public.dss");
    ifstream message;
    ifstream signature;
    Big p,q,g,y,v,u1,u2,r,s,h;
    char ifname[13],ofname[13];
    int bits;
    miracl *mip=&precision;

/* get public data */
    common >> bits;
    mip->IOBASE=16;
    common >> p >> q >> g;
    mip->IOBASE=10;
/* get public key of signer */
    public_key >> y;
/* get message */
    cout << "signed file = " ;
    cin.sync();
    cin.getline(ifname,13);
    strcpy(ofname,ifname);
    strip(ofname);
    strcat(ofname,".dss");
    message.open(ifname,ios::binary|ios::in); 
    if (!message)
    { /* no message */
        cout << "Unable to open file " << ifname << "\n";
        return 0;
    }
    h=hash(message);

    signature.open(ofname,ios::in);
    if (!signature)
    { /* no signature */
        cout << "signature file " << ofname << " does not exist\n";
        return 0;
    }
    signature >> r >> s;
    if (r>=q || s>=q)
    {
        cout << "Signature is NOT verified\n";
        return 0;
    }
    s=inverse(s,q);
    u1=(h*s)%q;
    u2=(r*s)%q;
    v=pow(g,u1,y,u2,p);  /* v=g^u1.y^u2 mod p */

    v%=q;
    if (v==r) cout << "Signature is verified\n";
    else      cout << "Signature is NOT verified\n";
    return 0;
}

