/* ggh-decrypt:
 * decrypt a file using the GGH cryptosystem
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


ZZ decrypt( int, mat_ZZ, mat_ZZ, vec_ZZ );


int main( int argc, char *argv[] )
{
    if ( argc < 4 ) {
        cerr << "Usage: ggh-decrypt <secretKeyFile> <cipherTextFile> <decipherTextFile>" << endl;
        exit(1);
    }

	char *secretKey = strdup( argv[1] );
	char *cipherText = strdup( argv[2] );
	char *decipherText = strdup( argv[3] );

	int mat_dim;
	mat_ZZ mat_private;
	mat_ZZ mat_unimodular;

	ifstream secretKeyFile;

	secretKeyFile.open( secretKey );
	secretKeyFile >> mat_dim;
	secretKeyFile >> mat_private;
	secretKeyFile >> mat_unimodular;
	secretKeyFile.close();

	cout << "GGH private dim   = " << mat_dim << endl;
	cout << "GGH private basis = " << endl << mat_private << endl;
	cout << "GGH unimodular    = " << endl << mat_unimodular << endl;

	ifstream cipherTextFile;
	ofstream decipherTextFile;

	cipherTextFile.open( cipherText );
	decipherTextFile.open( decipherText );

	vec_ZZ cipherVec;
	ZZ decipherNum;
	unsigned char outputChar;

	while ( !cipherTextFile.eof() ) {
		cipherTextFile >> cipherVec;
		cout << "cipherVec=" << cipherVec << endl;

		decipherNum = decrypt( mat_dim, mat_private, mat_unimodular, cipherVec );
		cout << "decipherNum=" << decipherNum << endl;

		BytesFromZZ( &outputChar, decipherNum, 1 );

		decipherTextFile.put( outputChar );
		cout << "outputChar=" << outputChar << endl;

		SkipWhiteSpace( cipherTextFile );
	}

	cipherTextFile.close();
	decipherTextFile.close();
}


/* decrypt the cipher which is a perturbed lattice point c. 
 * given c = (BU)m + e, apply B^{-1} to c and get y = Um + B^{-1}e.
 * then apply Babai's nearest-plane to get z = Um. apply U^{-1} to z to
 * obtain m.
 * BUGGY: perturbation is too big?
 */
ZZ decrypt( int n, mat_ZZ priv_basis, mat_ZZ unimodular_mask, vec_ZZ cipher_vec )
{
	ZZ det;
	mat_ZZ inv_basis;
	inv( det, inv_basis, priv_basis, 1 );

	/* apply basis reduction (Lenstra-Lenstra-Lovasz):
	ZZ det2;
	mat_ZZ reduced_basis;
	reduced_basis = priv_basis;
	LLL( det2, reduced_basis, 0 );
	*/

	vec_ZZ partial_vec;
	partial_vec = inv_basis * cipher_vec;

	vec_ZZ plain_vec;
	NearVector( plain_vec, priv_basis, partial_vec );

	ZZ det2;
	mat_ZZ inv_unimodular;
	inv( det2, inv_unimodular, unimodular_mask, 1 );

	plain_vec = inv_unimodular * plain_vec;

	ZZ plain_value = plain_vec[0] / (det * det2);

#ifdef DEBUG
	cout << "Dec(" << cipher_vec << " -> ";
	cout << plain_value << ")" << endl;
#endif

	return plain_value;
}




