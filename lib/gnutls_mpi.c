/*
 * Copyright (C) 2001,2002,2003 Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 *  The GNUTLS library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public   
 *  License as published by the Free Software Foundation; either 
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

/* Here lie everything that has to do with large numbers, libgcrypt and
 * other stuff that didn't fit anywhere else.
 */

#include <gnutls_int.h>
#include <libtasn1.h>
#include <gnutls_errors.h>
#include <gnutls_num.h>

/* Functions that refer to the libgcrypt library.
 */
 
void _gnutls_mpi_release( GNUTLS_MPI* x) {
	if (*x==NULL) return;
	gcry_mpi_release(*x);
	*x=NULL;
}

/* returns zero on success
 */
int _gnutls_mpi_scan( GNUTLS_MPI *ret_mpi, const opaque *buffer, size_t *nbytes ) {
	int ret;
	
	ret = gcry_mpi_scan( ret_mpi, GCRYMPI_FMT_USG, buffer, nbytes);
	if (ret) return ret;
	
	/* MPIs with 0 bits are illegal
	 */
	if (_gnutls_mpi_get_nbits( *ret_mpi) == 0) {
		_gnutls_mpi_release( ret_mpi);
		return 1;
	}

	return 0;
}

int _gnutls_mpi_scan_pgp( GNUTLS_MPI *ret_mpi, const opaque *buffer, size_t *nbytes ) {
int ret;
	ret = gcry_mpi_scan( ret_mpi, GCRYMPI_FMT_PGP, buffer, nbytes);

	if (ret) return ret;

	/* MPIs with 0 bits are illegal
	 */
	if (_gnutls_mpi_get_nbits( *ret_mpi) == 0) {
		_gnutls_mpi_release( ret_mpi);
		return 1;
	}

	return 0;
}

int _gnutls_mpi_print( opaque *buffer, size_t *nbytes, const GNUTLS_MPI a ) {
	return gcry_mpi_print( GCRYMPI_FMT_USG, buffer, nbytes, a);
}

/* Always has the first bit zero */
int _gnutls_mpi_print_lz( opaque *buffer, size_t *nbytes, const GNUTLS_MPI a ) {
	return gcry_mpi_print( GCRYMPI_FMT_STD, buffer, nbytes, a);
}


/* this function reads an integer
 * from asn1 structs. Combines the read and mpi_scan
 * steps.
 */
int _gnutls_x509_read_int( ASN1_TYPE node, const char* value, 
	GNUTLS_MPI* ret_mpi)
{
int result;
size_t s_len;
opaque* tmpstr = NULL;
int tmpstr_size;

	tmpstr_size = 0;
	result = asn1_read_value( node, value, NULL, &tmpstr_size);
	if (result != ASN1_MEM_ERROR) {
		gnutls_assert();
		return _gnutls_asn2err(result);
	}

	tmpstr = gnutls_alloca( tmpstr_size);
	if (tmpstr == NULL) {
		gnutls_assert();
		return GNUTLS_E_MEMORY_ERROR;
	}

	result = asn1_read_value( node, value, tmpstr, &tmpstr_size);
	if (result != ASN1_SUCCESS) {
		gnutls_assert();
		gnutls_afree( tmpstr);
		return _gnutls_asn2err(result);
	}

	s_len = tmpstr_size;
	if (_gnutls_mpi_scan( ret_mpi, tmpstr, &s_len) != 0) {
		gnutls_assert();
		gnutls_afree( tmpstr);
		return GNUTLS_E_MPI_SCAN_FAILED;
	}

	gnutls_afree( tmpstr);

	return 0;
}

/* Writes the specified integer into the specified node.
 */
int _gnutls_x509_write_int( ASN1_TYPE node, const char* value, GNUTLS_MPI mpi, int lz)
{
opaque *tmpstr;
size_t s_len;
int result;

	s_len = 0;
	if (lz) result = _gnutls_mpi_print_lz( NULL, &s_len, mpi);
	else result = _gnutls_mpi_print( NULL, &s_len, mpi);

	tmpstr = gnutls_alloca( s_len);
	if (tmpstr == NULL) {
		gnutls_assert();
		return GNUTLS_E_MEMORY_ERROR;
	}

	if (lz) result = _gnutls_mpi_print_lz( tmpstr, &s_len, mpi);
	else result = _gnutls_mpi_print( tmpstr, &s_len, mpi);
	
	if (result != 0) {
		gnutls_assert();
		gnutls_afree( tmpstr);
		return GNUTLS_E_MPI_PRINT_FAILED;
	}

	result = asn1_write_value( node, value, tmpstr, s_len);

	gnutls_afree( tmpstr);

	if (result != ASN1_SUCCESS) {
		gnutls_assert();
		return _gnutls_asn2err(result);
	}

	return 0;
}

