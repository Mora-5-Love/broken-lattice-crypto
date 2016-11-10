/* ggh-encrypt:
 * encrypt a file using the GGH cryptosystem
 */

#include <NTL/mat_ZZ.h>
#include <NTL/HNF.h>
#include <NTL/LLL.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <fstream>


using namespace std;
using namespace NTL;


vec_ZZ encrypt( int, mat_ZZ, ZZ );


int main( int argc, char *argv[] )
{
	if ( argc < 4 ) {
		cerr << "Usage: ggh-encrypt <publicKeyFile> <plainTextFile> <cipherTextFile>" << endl; 
		exit(1); 
	}

	char *publicKey = strdup( argv[1] );
	char *plainText = strdup( argv[2] );
	char *cipherText = strdup( argv[3] );

	int mat_dim;
	mat_ZZ mat_public;

	ifstream publicKeyFile;

	publicKeyFile.open( publicKey );
	publicKeyFile >> mat_dim;
	publicKeyFile >> mat_public;
	publicKeyFile.close();

	cout << "GGH public dim   = " << mat_dim << endl;
	cout << "GGH public basis = " << endl << mat_public << endl;

	ifstream plainTextFile;
	ofstream cipherTextFile;

	plainTextFile.open( plainText );
	cipherTextFile.open( cipherText );

	char inputChar;
	ZZ plainNum;
	vec_ZZ cipherNum;

	srand( time(0) );
	while ( !plainTextFile.eof() ) {
		plainTextFile.get( inputChar );
		plainNum = (long) inputChar;
		cipherNum = encrypt( mat_dim, mat_public, ZZ(plainNum) );
		cipherTextFile << cipherNum << endl;
	}

	plainTextFile.close();
	cipherTextFile.close();
}


/* encrypt a message that is a large integer as a perturbed lattice point.  
 * the message is first mapped into a vector which represents the integer
 * linear combination of the basis vectors.
 */
vec_ZZ encrypt( int n, mat_ZZ public_basis, ZZ msg )
{
	vec_ZZ plain_vec;
	plain_vec.SetLength( n );

	for (int j=0; j < n; j++) {
		plain_vec[j] = msg;
	}

	/* random perturbation */
	vec_ZZ delta;
	delta.SetLength( n );

	for (int j=0; j < n; j++) {
#ifdef PERTURB
		delta[j] = (rand() % 2 ? to_ZZ(1) : to_ZZ(-1));
#else
		delta[j] = to_ZZ(0);
#endif
	}

	vec_ZZ cipher_vec;
	cipher_vec = public_basis * plain_vec + delta;

#ifdef DEBUG1
	cout << "Enc(" << plain_vec << " -> ";
	cout << cipher_vec << ")" << endl;
#endif

	return cipher_vec;
}




