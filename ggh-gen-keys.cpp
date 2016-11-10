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


/* constants */
#define MAX_ENTRY_VALUE	4



int main( int argc, char *argv[] )
{
	if ( argc < 4 ) {
		cerr << "Usage: ggh-gen-keys <matrixDim> <publicKeyFile> <secretKeyFile>" << endl;
		exit(1);
	}

	cout << "GGH KEY GENERATOR" << endl;

	int mat_dim = atoi( argv[1] );

	vector< mat_ZZ > mat_private = gen_private_basis( mat_dim, MAX_ENTRY_VALUE );
	mat_ZZ mat_public = gen_public_basis( mat_private[0], mat_private[1] );

	char *publicKeyFileName;
	ofstream publicKeyFile;

	publicKeyFileName = strdup( argv[2] );
	cout << "Writing GGH public keys ..." << endl;
	publicKeyFile.open( publicKeyFileName );
	publicKeyFile << mat_dim << endl;
	publicKeyFile << mat_public << endl;
	publicKeyFile.close();

#ifdef DEBUG
	cout << mat_dim << endl;
	cout << mat_public << endl;
#endif

	char *secretKeyFileName;
	ofstream secretKeyFile;

	secretKeyFileName = strdup( argv[3] );
	cout << "Writing GGH secret keys ..." << endl;
	secretKeyFile.open( secretKeyFileName );
	secretKeyFile << mat_dim << endl;
	secretKeyFile << mat_private[0] << endl;
	secretKeyFile << mat_private[1] << endl;
	secretKeyFile.close();

#ifdef DEBUG
	cout << mat_dim << endl;
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
	/* generate nice basis */

	mat_ZZ mat;
	mat.SetDims( dim, dim );

	vec_ZZ vec;
	vec.SetLength( dim );

	srand( time(0) );
	SetSeed( to_ZZ( rand() ));

	for (int j = 0; j < dim; j++) {
		for (int k = 0; k < dim; k++) {
			vec[k] = (RandomBnd( 2*bound ) - to_ZZ( bound ));
		}
		mat[j] = vec;
	}

	mat_ZZ nice_basis = ident_mat_ZZ( dim );
	nice_basis = nice_basis * to_ZZ( sqrt(dim) );
	nice_basis = nice_basis + mat;


	/* generate random unimodular mask */
	mat_ZZ unimodular_mat;
	unimodular_mat = gen_random_unimodular( dim );

	vector< mat_ZZ > priv_keys;
	priv_keys.push_back( nice_basis );
	priv_keys.push_back( unimodular_mat );

#ifdef DEBUG
	/* check private basis and unimodular mask */
	cout << "Private basis = " << endl << nice_basis << endl;
	ZZ det = determinant( nice_basis, 1 );
	cout << "Private determinant = " << det << endl;
	assert( det != 0 );
	cout << "Unimodular mask = " << endl << unimodular_mat << "\n\n";
#endif

	return priv_keys;
}


/* gen_public_hnf_basis:
 * generates a public basis for the lattice, by using the
 * converting to a Hermite Normal Form.
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


/* gen_public_basis:
 * generates a public basis for the lattice, by applying a random
 * unimodular transformation to the private basis.
 */
mat_ZZ gen_public_basis( mat_ZZ private_basis, mat_ZZ unimodular_mat )
{

	ZZ priv_det;
	determinant( priv_det, private_basis, 1 );

	mat_ZZ public_basis;
	public_basis = private_basis * unimodular_mat;

#ifdef DEBUG
	/* check public basis */
	cout << "Public basis = " << endl << public_basis << endl;
	ZZ pub_det = determinant( public_basis, 1 );
	cout << "Public determinant = " << pub_det << "\n\n";
#endif

#ifdef DEBUG1
	/* check inverse of public basis */
	ZZ det;
	mat_ZZ matInv;
	inv( det, matInv, public_basis, 1 );
	cout << "Inverse = " << matInv << endl;
	cout << "Check: " << matInv * public_basis << endl;
#endif

	return public_basis;
}


/* gen_random_unimodular
 * generates a random unimodular matrix.
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


/* generate a random lower triangular matrix */
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


/* generate a random upper triangular matrix */
mat_ZZ gen_random_upper_triangular( int n )
{
	mat_ZZ lower;
	lower = gen_random_lower_triangular( n );

	mat_ZZ upper;
	upper = transpose( lower );

	return upper;
}




