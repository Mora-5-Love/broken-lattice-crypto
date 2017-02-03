/* ggh-gen-keys:
 * generate public and secret keys for the GGH cryptosystem
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
#include <vector>


using namespace std;
using namespace NTL;


/* prototypes */
vector< mat_ZZ > gen_private_basis( int, int );
mat_ZZ gen_public_basis( mat_ZZ, mat_ZZ );
mat_ZZ gen_random_unimodular( int );
mat_ZZ gen_random_lower_triangular( int );
mat_ZZ gen_random_upper_triangular( int );
RR orthodefect( mat_ZZ );


/* constants */
#define MAX_ENTRY_VALUE	4



int main( int argc, char *argv[] )
{
	if ( argc < 4 ) {
		cerr << "Usage: ggh-gen-keys <dim> <publicKeyFile> <secretKeyFile>" << endl;
		exit(1);
	}

	cout << "GGH KEY GENERATOR" << endl;

	int dim = atoi( argv[1] );

	vector< mat_ZZ > mat_private = gen_private_basis( dim, dim );
	mat_ZZ mat_public = gen_public_basis( mat_private[0], mat_private[1] );

	char *publicKeyFileName;
	ofstream publicKeyFile;

	publicKeyFileName = strdup( argv[2] );
	cout << "Writing GGH public keys ..." << endl;
	publicKeyFile.open( publicKeyFileName );
	publicKeyFile << dim << endl;
	publicKeyFile << mat_public << endl;
	publicKeyFile.close();

#ifdef DEBUG2
	cout << dim << endl;
	cout << mat_public << endl;
#endif

	char *secretKeyFileName;
	ofstream secretKeyFile;

	secretKeyFileName = strdup( argv[3] );
	cout << "Writing GGH secret keys ..." << endl;
	secretKeyFile.open( secretKeyFileName );
	secretKeyFile << dim << endl;
	secretKeyFile << mat_private[0] << endl;
	secretKeyFile << mat_private[1] << endl;
	secretKeyFile.close();

#ifdef DEBUG2
	cout << dim << endl;
	cout << mat_private[0] << endl;
	cout << mat_private[1] << endl;
#endif
	
}


/* gen_private_basis:
 * generates a nice basis B for the lattice, by using a random matrix
 * whose entries are chosen from the interval -k to +k, and a random
 * unimodular matrix U.
 */
vector< mat_ZZ > gen_private_basis( int dim, int bound )
{
	mat_ZZ private_basis;
	private_basis.SetDims( dim, dim );

	/* use random matrix */
	srand( time(0) );
	SetSeed( to_ZZ( rand() ));
	for (int j = 0; j < dim; j++) {
		for (int k = 0; k < dim; k++) {
			private_basis[j][k] = (RandomBnd( 2*bound ) - to_ZZ( bound ));
		}
	}

#ifdef DIAG_DOMINANT
	/* optional: add large constant diagonal value */
	ZZ diag_value = RoundToZZ( sqrt( MakeRR( to_ZZ( dim ), 0 ))) * to_ZZ( bound );
	for (int j = 0; j < dim; j++) {
		private_basis[j][j] += diag_value;
	}
#endif


#ifdef APPLY_LLL
	/* apply LLL to reduce the basis */
	cout << "PreLLL:\n" << private_basis << endl;
	ZZ det_reduced;
	LLL( det_reduced, private_basis, 0 );
	cout << "PostLLL:\n" << private_basis << endl;
#endif


	/* generate random unimodular matrix 
	 * to obfuscate private basis into public basis */
	mat_ZZ unimodular_mask;
	unimodular_mask = gen_random_unimodular( dim );

	vector< mat_ZZ > priv_keys;
	priv_keys.push_back( private_basis );
	priv_keys.push_back( unimodular_mask );

#ifdef DEBUG1
	/* check private basis and unimodular mask */
	cout << "Private basis = " << endl << private_basis << endl;
	ZZ det_priv = determinant( private_basis, 1 );
	cout << "Orthodefect = " << orthodefect( private_basis ) << endl;
	cout << "Private determinant = " << det_priv << endl;
	assert( det_priv != 0 );
	cout << "Unimodular mask = " << endl << unimodular_mask << "\n\n";
#endif

	return priv_keys;
}


/* gen_public_basis:
 * generates a public basis for the lattice, by applying a random
 * unimodular transformation to the private basis.
 */
mat_ZZ gen_public_basis( mat_ZZ private_basis, mat_ZZ unimodular_mask )
{
	mat_ZZ public_basis;
	public_basis = private_basis * unimodular_mask;

#ifdef DEBUG1
	/* check public basis */
	cout << "Public basis = " << endl << public_basis << endl;
	cout << "Orthodefect = " << orthodefect( public_basis ) << endl;
	ZZ pub_det = determinant( public_basis, 1 );
	cout << "Public determinant = " << pub_det << endl;
#endif

#ifdef DEBUG2
	/* check inverse of public basis */
	ZZ det;
	mat_ZZ matInv;
	inv( det, matInv, public_basis, 1 );
	cout << "Inverse = " << matInv << endl;
	cout << "Check: " << matInv * public_basis << endl;
#endif

	return public_basis;
}


/* generates a random unimodular matrix with determinant +1,-1.
 */
mat_ZZ gen_random_unimodular( int n )
{
	mat_ZZ lower;
	lower = gen_random_lower_triangular( n );

	mat_ZZ upper;
	upper = gen_random_upper_triangular( n );

	mat_ZZ mat;
	mat = lower * upper;

	return mat;
}


/* generate a random lower triangular matrix.
 * the diagonal entries are +1,-1 and the lower nonzero entries are 0,+1,-1.
 */
mat_ZZ gen_random_lower_triangular( int n )
{
	mat_ZZ lower;
	lower.SetDims( n, n );

	int ternary_bit;
	for (int j=0; j<n; j++) 
	{
		for (int k=0; k<j; k++) 
		{
			ternary_bit = (rand() % 3);
			lower[j][k] = to_ZZ( ternary_bit - 1 );
		}

		lower[j][j] = ( rand() % 2 ? 1 : -1);

		for (int k=j+1; k<n; k++) 
		{
			lower[j][k] = to_ZZ( 0 );
		}
	}

	return lower;
}


/* generate a random upper triangular matrix.
 * the diagonal entries are +1,-1 and the upper nonzero entries are 0,+1,-1.
 */
mat_ZZ gen_random_upper_triangular( int n )
{
	mat_ZZ lower;
	lower = gen_random_lower_triangular( n );

	mat_ZZ upper;
	upper = transpose( lower );

	return upper;
}


/* gen_public_hnf_basis: (alternative method suggested by D. Micciancio)
 * generates a public basis for the lattice, by using the Hermite Normal Form.
 */
mat_ZZ gen_public_hnf_basis( mat_ZZ private_basis )
{
	ZZ priv_det;
	determinant( priv_det, private_basis, 1 );

	mat_ZZ public_basis;
	HNF( public_basis, private_basis, priv_det );

#ifdef DEBUG1
	/* check public basis */
	cout << "Public basis = " << endl << public_basis << endl;
	ZZ pub_det = determinant( public_basis, 1 );
	cout << "Public determinant = " << pub_det << "\n\n";
#endif

#ifdef DEBUG1
	/* invertible? */
	ZZ det;
	mat_ZZ matInv;
	inv( det, matInv, public_basis, 1 );
	cout << "Inverse = " << matInv << endl;
	cout << "Check: " << matInv * public_basis << endl;
#endif

	return public_basis;
}



/* computes the orthogonality defect of a basis */
RR orthodefect( mat_ZZ basis )
{
	int size = basis.NumRows();
	ZZ norm_squared;

	RR value = MakeRR( to_ZZ(1), 0 );

	for (int j = 0; j < size; j++) {
		InnerProduct( norm_squared, basis[j], basis[j] );
		value *= SqrRoot( MakeRR( norm_squared, 0 ));
	}

	ZZ det_basis = determinant( basis );

	return abs( value / MakeRR( det_basis, 0 ));
}






