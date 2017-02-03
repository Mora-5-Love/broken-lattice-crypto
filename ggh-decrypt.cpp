/* ggh-decrypt:
 * decrypt a file using the GGH cryptosystem
 */

#include <NTL/matrix.h>
#include <NTL/mat_ZZ.h>
#include <NTL/vec_ZZ.h>
#include <NTL/mat_RR.h>
#include <NTL/vec_RR.h>
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


/* prototype */
ZZ decrypt( int, mat_ZZ, mat_ZZ, vec_ZZ );
ZZ decrypt_with_nearest_plane( int, mat_ZZ, mat_ZZ, vec_ZZ );
ZZ decrypt_with_rounding( int, mat_ZZ, mat_ZZ, vec_ZZ );



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

#ifdef DEBUG2
	cout << "GGH private dim   = " << mat_dim << endl;
	cout << "GGH private basis = " << endl << mat_private << endl;
	cout << "GGH unimodular    = " << endl << mat_unimodular << endl;
#endif

	ifstream cipherTextFile;
	ofstream decipherTextFile;

	cipherTextFile.open( cipherText );
	decipherTextFile.open( decipherText );

	vec_ZZ cipherVec;
	ZZ decipherNum;
	unsigned char outputChar;

	while ( !cipherTextFile.eof() ) {
		cipherTextFile >> cipherVec;
		cout << "Vec=" << cipherVec << endl;

		decipherNum = decrypt( mat_dim, mat_private, mat_unimodular, cipherVec );
		cout << "Num=[" << decipherNum << "]" << endl;

		BytesFromZZ( &outputChar, decipherNum, 1 );

		decipherTextFile.put( outputChar );
		cout << "Char=[" << outputChar << "]" << endl;

		SkipWhiteSpace( cipherTextFile );
	}

	cipherTextFile.close();
	decipherTextFile.close();
}


/* decrypt the cipher */
ZZ decrypt( int n, mat_ZZ priv_basis, mat_ZZ unimodular_mask, vec_ZZ cipher_vec )
{
#ifdef BABAI_ROUNDING
	return decrypt_with_rounding( n, priv_basis, unimodular_mask, cipher_vec ); 
#else
	return decrypt_with_nearest_plane( n, priv_basis, unimodular_mask, cipher_vec ); 
#endif
}


/* decrypt the cipher which is a perturbed lattice point c. 
 *  given c = (BU)m + e, apply B^{-1} to c and get y = Um + B^{-1}e.
 *  apply Babai's nearest-plane algorithm to get z = Um. apply U^{-1} to z to get m.
 * BUGGY: perturbation is too big?
 * ALTERNATIVE: swap the steps 
 *  given c = (BU)m + e, find nearest vector y = (BU)m. compute m = U^{-1}B^{-1}y.
 *  status: this did not even work for the case without perturbation.
 */
ZZ decrypt_with_nearest_plane( int n, mat_ZZ priv_basis, mat_ZZ unimodular_mask, vec_ZZ cipher_vec )
{
	/* step 1: canceling private basis */
	ZZ priv_det;
	mat_ZZ inv_priv_basis;
	inv( priv_det, inv_priv_basis, priv_basis, 1 );

	vec_ZZ plain_vec;
	plain_vec = inv_priv_basis * cipher_vec;

	/* step 2: apply Babai's nearest-plane algorithm */
	vec_ZZ closest_vec;
	NearVector( closest_vec, priv_basis, plain_vec );

	/* step 3: canceling unimodular mask */
	ZZ umod_det;
	mat_ZZ inv_unimodular_mask;
	inv( umod_det, inv_unimodular_mask, unimodular_mask, 1 );

	closest_vec = inv_unimodular_mask * closest_vec;

#ifdef DEBUG
	cout << "DEC[" << cipher_vec << " -> " << closest_vec << "]" << endl;
#endif

	return closest_vec[0] / (priv_det * umod_det);
}



/* decrypt the cipher which is a perturbed lattice point c. 
 * given c = (BU)m + e, apply B^{-1} to c and get y = Um + B^{-1}e.
 * apply Babai's rounding to get z = Um. apply U^{-1} to z to get m.
 * BUGGY: perturbation is too big?
 */
ZZ decrypt_with_rounding( int n, mat_ZZ priv_basis, mat_ZZ unimodular_mask, vec_ZZ cipher_vec )
{
	ZZ det_priv;
	mat_ZZ inv_priv_basis;
	inv( det_priv, inv_priv_basis, priv_basis, 1 );

	/* convert to real cipher */
	vec_RR cipher_rvec;
	cipher_rvec.SetLength( n );
	for (int j = 0; j < n; j++) {
		cipher_rvec[j] = MakeRR( cipher_vec[j], 0 );
	}

	/* apply inverse of private basis */
	vec_RR plain_rvec;
	mat_RR inv_rbasis;

	// this didn't fly. must do by-hand.
	//inv_priv_rbasis = conv< Mat<RR> >( inv_priv_basis );

	for (int j = 0; j < n; j++) {
		for (int k = 0; k < n; k++) {
			inv_rbasis[j][k] = MakeRR( inv_priv_basis[j][k], 0 );
		}
	}

	plain_rvec = inv_rbasis * cipher_rvec;

	for (int j = 0; j < n; j++) {
		plain_rvec[j] = plain_rvec[j] / MakeRR( det_priv, 0 );
	}

	/* round to integer */
	vec_ZZ plain_vec;
	plain_vec.SetLength( n );
	for (int j = 0; j < n; j++) {
		plain_vec[j] = RoundToZZ( plain_rvec[j] );	/* correct negatives? */
	}

	/* cancel unimodular mask */
	ZZ det_unimod;
	mat_ZZ inv_unimodular;
	inv( det_unimod, inv_unimodular, unimodular_mask, 1 );

	plain_vec = inv_unimodular * plain_vec;

	/* determinant of the unimodular mask is +1 or -1 */
	for (int j = 0; j < n; j++) {
		plain_vec[j] = det_unimod * plain_vec[j];
	}

#ifdef DEBUG
	cout << "DEC[" << cipher_vec << " -> " << plain_vec[0] << "]" << endl;
#endif

	return plain_vec[0];
}





