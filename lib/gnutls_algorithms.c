/*
 *      Copyright (C) 2000 Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "gnutls_int.h"
#include "gnutls_algorithms.h"
#include "gnutls_errors.h"
#include "gnutls_cert.h"

#define MAX_CIPHER 256
#define MAX_MAC 256
#define MAX_KX 256
#define MAX_CIPHERSUITE 256
#define MAX_COMPRESSION 256
#define MAX_VERSION 256


/* Cred type mappings to KX algorithms */
typedef struct {
	KXAlgorithm algorithm;
	CredType type;
} gnutls_cred_map;

static const gnutls_cred_map cred_mappings[] = {
	{ GNUTLS_KX_ANON_DH, GNUTLS_ANON    },
	{ GNUTLS_KX_X509PKI_RSA,     GNUTLS_X509PKI },
	{ GNUTLS_KX_X509PKI_DHE_DSS, GNUTLS_X509PKI },
	{ GNUTLS_KX_X509PKI_DHE_RSA, GNUTLS_X509PKI },
	{ GNUTLS_KX_SRP,     GNUTLS_SRP     },
	{ 0 }
};

#define GNUTLS_KX_MAP_LOOP(b) \
        const gnutls_cred_map *p; \
                for(p = cred_mappings; p->type != 0; p++) { b ; }

#define GNUTLS_KX_MAP_ALG_LOOP(a) \
                        GNUTLS_KX_MAP_LOOP( if(p->type == type) { a; break; })


/* TLS Versions */

typedef struct {
	char *name;
	GNUTLS_Version id;	/* gnutls internal version number */
	int major;		/* defined by the protocol */
	int minor;		/* defined by the protocol */
	int supported;		/* 0 not supported, > 0 is supported */
} gnutls_version_entry;

static const gnutls_version_entry sup_versions[] = {
	{"SSL 3.0", GNUTLS_SSL3, 3, 0, 1},
	{"TLS 1.0", GNUTLS_TLS1, 3, 1, 1},
	{"UNKNOWN", GNUTLS_VERSION_UNKNOWN, 0, 0, 1},
	{0}
};

#define GNUTLS_VERSION_LOOP(b) \
        const gnutls_version_entry *p; \
                for(p = sup_versions; p->name != NULL; p++) { b ; }

#define GNUTLS_VERSION_ALG_LOOP(a) \
                        GNUTLS_VERSION_LOOP( if(p->id == version) { a; break; })


#define GNUTLS_CIPHER_ENTRY(name, blksize, keysize, block, iv) \
	{ #name, name, blksize, keysize, block, iv }

struct gnutls_cipher_entry {
	char *name;
	BulkCipherAlgorithm id;
	size_t blocksize;
	size_t keysize;
	CipherType block;
	size_t iv;
};
typedef struct gnutls_cipher_entry gnutls_cipher_entry;

/* Note that all algorithms are in CBC or STREAM modes. 
 * Do not add any algorithms in other modes (avoid modified algorithms).
 * View first: "The order of encryption and authentication for
 * protecting communications" by Hugo Krawczyk - CRYPTO 2001
 */
static const gnutls_cipher_entry algorithms[] = {
	GNUTLS_CIPHER_ENTRY(GNUTLS_CIPHER_3DES_CBC, 8, 24, CIPHER_BLOCK, 8),
	GNUTLS_CIPHER_ENTRY(GNUTLS_CIPHER_RIJNDAEL_CBC, 16, 16, CIPHER_BLOCK, 16),
	GNUTLS_CIPHER_ENTRY(GNUTLS_CIPHER_RIJNDAEL256_CBC, 16, 32, CIPHER_BLOCK, 16),
	GNUTLS_CIPHER_ENTRY(GNUTLS_CIPHER_TWOFISH_CBC, 16, 16, CIPHER_BLOCK, 16),
	GNUTLS_CIPHER_ENTRY(GNUTLS_CIPHER_ARCFOUR, 1, 16, CIPHER_STREAM, 0),
	GNUTLS_CIPHER_ENTRY(GNUTLS_CIPHER_NULL, 1, 0, CIPHER_STREAM, 0),
	{0}
};

#define GNUTLS_LOOP(b) \
        const gnutls_cipher_entry *p; \
                for(p = algorithms; p->name != NULL; p++) { b ; }

#define GNUTLS_ALG_LOOP(a) \
                        GNUTLS_LOOP( if(p->id == algorithm) { a; break; } )


#define GNUTLS_HASH_ENTRY(name, hashsize) \
	{ #name, name, hashsize }

struct gnutls_hash_entry {
	char *name;
	MACAlgorithm id;
	size_t digestsize;
};
typedef struct gnutls_hash_entry gnutls_hash_entry;

static const gnutls_hash_entry hash_algorithms[] = {
	GNUTLS_HASH_ENTRY(GNUTLS_MAC_SHA, 20),
	GNUTLS_HASH_ENTRY(GNUTLS_MAC_MD5, 16),
	GNUTLS_HASH_ENTRY(GNUTLS_MAC_NULL, 0),
	{0}
};

#define GNUTLS_HASH_LOOP(b) \
        const gnutls_hash_entry *p; \
                for(p = hash_algorithms; p->name != NULL; p++) { b ; }

#define GNUTLS_HASH_ALG_LOOP(a) \
                        GNUTLS_HASH_LOOP( if(p->id == algorithm) { a; break; } )


/* Compression Section */
#define GNUTLS_COMPRESSION_ENTRY(name, id) \
	{ #name, name, id }

struct gnutls_compression_entry {
	char *name;
	CompressionMethod id;
	int num; /* the number reserved in TLS for the specific compression method */
};

typedef struct gnutls_compression_entry gnutls_compression_entry;
static const gnutls_compression_entry compression_algorithms[] = {
	GNUTLS_COMPRESSION_ENTRY(GNUTLS_COMP_NULL, 0),
#ifdef HAVE_LIBZ
	GNUTLS_COMPRESSION_ENTRY(GNUTLS_COMP_ZLIB, 224),
#endif
	{0}
};

#define GNUTLS_COMPRESSION_LOOP(b) \
        const gnutls_compression_entry *p; \
                for(p = compression_algorithms; p->name != NULL; p++) { b ; }
#define GNUTLS_COMPRESSION_ALG_LOOP(a) \
                        GNUTLS_COMPRESSION_LOOP( if(p->id == algorithm) { a; break; } )
#define GNUTLS_COMPRESSION_ALG_LOOP_NUM(a) \
                        GNUTLS_COMPRESSION_LOOP( if(p->num == num) { a; break; } )


/* Key Exchange Section */
#define GNUTLS_KX_ALGO_ENTRY(name, auth_struct) \
	{ #name, name, auth_struct }

struct gnutls_kx_algo_entry {
	char *name;
	KXAlgorithm algorithm;
	MOD_AUTH_STRUCT *auth_struct;
};
typedef struct gnutls_kx_algo_entry gnutls_kx_algo_entry;

extern MOD_AUTH_STRUCT rsa_auth_struct;
extern MOD_AUTH_STRUCT dhe_rsa_auth_struct;
extern MOD_AUTH_STRUCT anon_auth_struct;
extern MOD_AUTH_STRUCT srp_auth_struct;

static const gnutls_kx_algo_entry kx_algorithms[] = {
	GNUTLS_KX_ALGO_ENTRY(GNUTLS_KX_ANON_DH, &anon_auth_struct),
	GNUTLS_KX_ALGO_ENTRY(GNUTLS_KX_X509PKI_RSA, &rsa_auth_struct),
	GNUTLS_KX_ALGO_ENTRY(GNUTLS_KX_X509PKI_DHE_RSA, &dhe_rsa_auth_struct),
	GNUTLS_KX_ALGO_ENTRY(GNUTLS_KX_SRP, &srp_auth_struct),
	{0}
};

#define GNUTLS_KX_LOOP(b) \
        const gnutls_kx_algo_entry *p; \
                for(p = kx_algorithms; p->name != NULL; p++) { b ; }

#define GNUTLS_KX_ALG_LOOP(a) \
                        GNUTLS_KX_LOOP( if(p->algorithm == algorithm) { a; break; } )



/* Cipher SUITES */
#define GNUTLS_CIPHER_SUITE_ENTRY( name, block_algorithm, kx_algorithm, mac_algorithm ) \
	{ #name, {name}, block_algorithm, kx_algorithm, mac_algorithm }

typedef struct {
	char *name;
	GNUTLS_CipherSuite id;
	BulkCipherAlgorithm block_algorithm;
	KXAlgorithm kx_algorithm;
	MACAlgorithm mac_algorithm;
} gnutls_cipher_suite_entry;

#define GNUTLS_X509PKI_RSA_NULL_MD5 { 0x00, 0x01 }

#define GNUTLS_ANON_DH_3DES_EDE_CBC_SHA { 0x00, 0x1B }
#define GNUTLS_ANON_DH_ARCFOUR_MD5 { 0x00, 0x18 }
#define GNUTLS_ANON_DH_RIJNDAEL_128_CBC_SHA { 0x00, 0x34 }
#define GNUTLS_ANON_DH_RIJNDAEL_256_CBC_SHA { 0x00, 0x3A }
/* Twofish is a gnutls extension */
#define GNUTLS_ANON_DH_TWOFISH_128_CBC_SHA { 0xF6, 0x50 }

/* SRP is a gnutls extension - for now */
#define GNUTLS_SRP_3DES_EDE_CBC_SHA { 0x00, 0x5B }
#define GNUTLS_SRP_ARCFOUR_SHA { 0x00, 0x5C }
#define GNUTLS_SRP_ARCFOUR_MD5 { 0x00, 0x5F }
#define GNUTLS_SRP_RIJNDAEL_128_CBC_SHA { 0xF6, 0x62 }
#define GNUTLS_SRP_RIJNDAEL_256_CBC_SHA { 0xF6, 0x63 }
#define GNUTLS_SRP_TWOFISH_128_CBC_SHA { 0xF6, 0x64 }

/* RSA */
#define GNUTLS_X509PKI_RSA_ARCFOUR_SHA { 0x00, 0x05 }
#define GNUTLS_X509PKI_RSA_ARCFOUR_MD5 { 0x00, 0x04 }
#define GNUTLS_X509PKI_RSA_3DES_EDE_CBC_SHA { 0x00, 0x0A }
#define GNUTLS_X509PKI_RSA_DES_CBC_SHA { 0x00, 0x09 }
#define GNUTLS_X509PKI_RSA_RIJNDAEL_128_CBC_SHA { 0x00, 0x2F }
#define GNUTLS_X509PKI_RSA_RIJNDAEL_256_CBC_SHA { 0x00, 0x35 }
#define GNUTLS_X509PKI_RSA_TWOFISH_128_CBC_SHA { 0xF6, 0x51 }

/* X509PKI_DHE_DSS */
#define GNUTLS_X509PKI_DHE_DSS_RIJNDAEL_256_CBC_SHA { 0x00, 0x38 }
#define GNUTLS_X509PKI_DHE_DSS_RIJNDAEL_128_CBC_SHA { 0x00, 0x32 }
#define GNUTLS_X509PKI_DHE_DSS_DES_CBC_SHA { 0x00, 0x12 }
#define GNUTLS_X509PKI_DHE_DSS_TWOFISH_128_CBC_SHA { 0xF6, 0x54 }
#define GNUTLS_X509PKI_DHE_DSS_3DES_EDE_CBC_SHA { 0x00, 0x13 }

/* X509PKI_DHE_RSA */
#define GNUTLS_X509PKI_DHE_RSA_TWOFISH_128_CBC_SHA { 0xF6, 0x55 }
#define GNUTLS_X509PKI_DHE_RSA_3DES_EDE_CBC_SHA { 0x00, 0x16 }
#define GNUTLS_X509PKI_DHE_RSA_DES_CBC_SHA { 0x00, 0x15 }
#define GNUTLS_X509PKI_DHE_RSA_RIJNDAEL_128_CBC_SHA { 0x00, 0x33 }
#define GNUTLS_X509PKI_DHE_RSA_RIJNDAEL_256_CBC_SHA { 0x00, 0x39 }

static const gnutls_cipher_suite_entry cs_algorithms[] = {
	/* ANON_DH */
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_ANON_DH_ARCFOUR_MD5,
				  GNUTLS_CIPHER_ARCFOUR,
				  GNUTLS_KX_ANON_DH, GNUTLS_MAC_MD5),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_ANON_DH_3DES_EDE_CBC_SHA,
				  GNUTLS_CIPHER_3DES_CBC, GNUTLS_KX_ANON_DH,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_ANON_DH_RIJNDAEL_128_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL_CBC, GNUTLS_KX_ANON_DH,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_ANON_DH_RIJNDAEL_256_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL256_CBC, GNUTLS_KX_ANON_DH,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_ANON_DH_TWOFISH_128_CBC_SHA,
				  GNUTLS_CIPHER_TWOFISH_CBC, GNUTLS_KX_ANON_DH,
				  GNUTLS_MAC_SHA),

	/* SRP */
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_SRP_ARCFOUR_SHA,
				  GNUTLS_CIPHER_ARCFOUR,
				  GNUTLS_KX_SRP, GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_SRP_ARCFOUR_MD5,
				  GNUTLS_CIPHER_ARCFOUR,
				  GNUTLS_KX_SRP, GNUTLS_MAC_MD5),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_SRP_3DES_EDE_CBC_SHA,
				  GNUTLS_CIPHER_3DES_CBC, GNUTLS_KX_SRP,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_SRP_RIJNDAEL_128_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL_CBC, GNUTLS_KX_SRP,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_SRP_RIJNDAEL_256_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL256_CBC, GNUTLS_KX_SRP,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_SRP_TWOFISH_128_CBC_SHA,
				  GNUTLS_CIPHER_TWOFISH_CBC, GNUTLS_KX_SRP,
				  GNUTLS_MAC_SHA),

	/* X509PKI_DHE_DSS */
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_DHE_DSS_TWOFISH_128_CBC_SHA,
				  GNUTLS_CIPHER_TWOFISH_CBC, GNUTLS_KX_X509PKI_DHE_DSS,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_DHE_DSS_3DES_EDE_CBC_SHA,
				  GNUTLS_CIPHER_3DES_CBC, GNUTLS_KX_X509PKI_DHE_DSS,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_DHE_DSS_RIJNDAEL_128_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL_CBC, GNUTLS_KX_X509PKI_DHE_DSS,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_DHE_DSS_RIJNDAEL_256_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL256_CBC, GNUTLS_KX_X509PKI_DHE_DSS,
				  GNUTLS_MAC_SHA),

	/* X509PKI_DHE_RSA */
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_DHE_RSA_TWOFISH_128_CBC_SHA,
				  GNUTLS_CIPHER_TWOFISH_CBC, GNUTLS_KX_X509PKI_DHE_RSA,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_DHE_RSA_3DES_EDE_CBC_SHA,
				  GNUTLS_CIPHER_3DES_CBC, GNUTLS_KX_X509PKI_DHE_RSA,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_DHE_RSA_RIJNDAEL_128_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL_CBC, GNUTLS_KX_X509PKI_DHE_RSA,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_DHE_RSA_RIJNDAEL_256_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL256_CBC, GNUTLS_KX_X509PKI_DHE_RSA,
				  GNUTLS_MAC_SHA),

	/* X509PKI_RSA */
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_RSA_NULL_MD5,
				  GNUTLS_CIPHER_NULL,
				  GNUTLS_KX_X509PKI_RSA, GNUTLS_MAC_MD5),

	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_RSA_ARCFOUR_SHA,
				  GNUTLS_CIPHER_ARCFOUR,
				  GNUTLS_KX_X509PKI_RSA, GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_RSA_ARCFOUR_MD5,
				  GNUTLS_CIPHER_ARCFOUR,
				  GNUTLS_KX_X509PKI_RSA, GNUTLS_MAC_MD5),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_RSA_3DES_EDE_CBC_SHA,
				  GNUTLS_CIPHER_3DES_CBC,
				  GNUTLS_KX_X509PKI_RSA, GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_RSA_RIJNDAEL_128_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL_CBC, GNUTLS_KX_X509PKI_RSA,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_RSA_RIJNDAEL_256_CBC_SHA,
				  GNUTLS_CIPHER_RIJNDAEL256_CBC, GNUTLS_KX_X509PKI_RSA,
				  GNUTLS_MAC_SHA),
	GNUTLS_CIPHER_SUITE_ENTRY(GNUTLS_X509PKI_RSA_TWOFISH_128_CBC_SHA,
				  GNUTLS_CIPHER_TWOFISH_CBC, GNUTLS_KX_X509PKI_RSA,
				  GNUTLS_MAC_SHA),

	{0}
};

#define GNUTLS_CIPHER_SUITE_LOOP(b) \
        const gnutls_cipher_suite_entry *p; \
                for(p = cs_algorithms; p->name != NULL; p++) { b ; }

#define GNUTLS_CIPHER_SUITE_ALG_LOOP(a) \
                        GNUTLS_CIPHER_SUITE_LOOP( if( (p->id.CipherSuite[0] == suite.CipherSuite[0]) && (p->id.CipherSuite[1] == suite.CipherSuite[1])) { a; break; } )



/* Generic Functions */

/* HASHES */
int _gnutls_mac_get_digest_size(MACAlgorithm algorithm)
{
	size_t ret = 0;
	GNUTLS_HASH_ALG_LOOP(ret = p->digestsize);
	return ret;

}

inline int _gnutls_mac_priority(GNUTLS_STATE state, MACAlgorithm algorithm)
{				/* actually returns the priority */
	int i;
	for (i = 0;
	     i < state->gnutls_internals.MACAlgorithmPriority.algorithms;
	     i++) {
		if (state->gnutls_internals.
		    MACAlgorithmPriority.algorithm_priority[i] ==
		    algorithm)
			return i;
	}
	return -1;
}

/**
  * gnutls_mac_get_name - Returns a string with the name of the specified mac algorithm
  * @algorithm: is a MAC algorithm
  *
  * Returns a string that contains the name 
  * of the specified MAC algorithm.
  **/
const char *gnutls_mac_get_name(MACAlgorithm algorithm)
{
	char *ret = NULL;

	/* avoid prefix */
	GNUTLS_HASH_ALG_LOOP(ret =
			     p->name + sizeof("GNUTLS_MAC_") - 1);

	return ret;
}

int _gnutls_mac_count()
{
	uint8 i, counter = 0;
	for (i = 0; i < MAX_MAC; i++) {
		if (_gnutls_mac_is_ok(i) == 0)
			counter++;
	}
	return counter;
}

int _gnutls_mac_is_ok(MACAlgorithm algorithm)
{
	size_t ret = -1;
	GNUTLS_HASH_ALG_LOOP(ret = p->id);
	if (ret >= 0)
		ret = 0;
	else
		ret = 1;
	return ret;
}

/* Compression Functions */
inline
    int _gnutls_compression_priority(GNUTLS_STATE state,
				     CompressionMethod algorithm)
{				/* actually returns the priority */
	int i;
	for (i = 0;
	     i <
	     state->gnutls_internals.CompressionMethodPriority.algorithms;
	     i++) {
		if (state->gnutls_internals.
		    CompressionMethodPriority.algorithm_priority[i] ==
		    algorithm)
			return i;
	}
	return -1;
}

/**
  * gnutls_compression_get_name - Returns a string with the name of the specified compression algorithm
  * @algorithm: is a Compression algorithm
  *
  * Returns a pointer to a string that contains the name 
  * of the specified compression algorithm.
  **/
const char *gnutls_compression_get_name(CompressionMethod algorithm)
{
	char *ret = NULL;

	/* avoid prefix */
	GNUTLS_COMPRESSION_ALG_LOOP(ret =
				    p->name + sizeof("GNUTLS_COMP_") -
					   1);

	return ret;
}

/* return the tls number of the specified algorithm */
int _gnutls_compression_get_num(CompressionMethod algorithm)
{
	int ret = -1;

	/* avoid prefix */
	GNUTLS_COMPRESSION_ALG_LOOP(ret = p->num);

	return ret;
}

/* returns the gnutls internal ID of the TLS compression
 * method num
 */
CompressionMethod _gnutls_compression_get_id(int num)
{
	CompressionMethod ret = -1;

	/* avoid prefix */
	GNUTLS_COMPRESSION_ALG_LOOP_NUM(ret = p->id);

	return ret;
}

int _gnutls_compression_count()
{
	uint8 i, counter = 0;
	for (i = 0; i < MAX_COMPRESSION; i++) {
		if (_gnutls_compression_is_ok(i) == 0)
			counter++;
	}
	return counter;
}

int _gnutls_compression_is_ok(CompressionMethod algorithm)
{
	size_t ret = -1;
	GNUTLS_COMPRESSION_ALG_LOOP(ret = p->id);
	if (ret >= 0)
		ret = 0;
	else
		ret = 1;
	return ret;
}



/* CIPHER functions */
int _gnutls_cipher_get_block_size(BulkCipherAlgorithm algorithm)
{
	size_t ret = 0;
	GNUTLS_ALG_LOOP(ret = p->blocksize);
	return ret;

}

 /* returns the priority */
inline
    int
_gnutls_cipher_priority(GNUTLS_STATE state, BulkCipherAlgorithm algorithm)
{
	int i;
	for (i = 0;
	     i <
	     state->gnutls_internals.
	     BulkCipherAlgorithmPriority.algorithms; i++) {
		if (state->gnutls_internals.
		    BulkCipherAlgorithmPriority.algorithm_priority[i] ==
		    algorithm)
			return i;
	}
	return -1;
}


int _gnutls_cipher_is_block(BulkCipherAlgorithm algorithm)
{
	size_t ret = 0;

	GNUTLS_ALG_LOOP(ret = p->block);
	return ret;

}

int _gnutls_cipher_get_key_size(BulkCipherAlgorithm algorithm)
{				/* In bytes */
	size_t ret = 0;
	GNUTLS_ALG_LOOP(ret = p->keysize);
	return ret;

}

int _gnutls_cipher_get_iv_size(BulkCipherAlgorithm algorithm)
{				/* In bytes */
	size_t ret = 0;
	GNUTLS_ALG_LOOP(ret = p->iv);
	return ret;

}

/**
  * gnutls_cipher_get_name - Returns a string with the name of the specified cipher algorithm
  * @algorithm: is an encryption algorithm
  *
  * Returns a pointer to a string that contains the name 
  * of the specified cipher.
  **/
const char *gnutls_cipher_get_name(BulkCipherAlgorithm algorithm)
{
	char *ret = NULL;

	/* avoid prefix */
	GNUTLS_ALG_LOOP(ret = p->name + sizeof("GNUTLS_CIPHER_") - 1);

	return ret;
}

int _gnutls_cipher_count()
{
	uint8 i, counter = 0;
	for (i = 0; i < MAX_CIPHER; i++) {
		if (_gnutls_cipher_is_ok(i) == 0)
			counter++;
	}
	return counter;
}


int _gnutls_cipher_is_ok(BulkCipherAlgorithm algorithm)
{
	size_t ret = -1;
	GNUTLS_ALG_LOOP(ret = p->id);
	if (ret >= 0)
		ret = 0;
	else
		ret = 1;
	return ret;
}


/* Key EXCHANGE functions */
MOD_AUTH_STRUCT *_gnutls_kx_auth_struct(KXAlgorithm algorithm)
{
	MOD_AUTH_STRUCT *ret = NULL;
	GNUTLS_KX_ALG_LOOP(ret = p->auth_struct);
	return ret;

}

inline int _gnutls_kx_priority(GNUTLS_STATE state, KXAlgorithm algorithm)
{
	int i;
	for (i = 0;
	     i < state->gnutls_internals.KXAlgorithmPriority.algorithms;
	     i++) {
		if (state->gnutls_internals.
		    KXAlgorithmPriority.algorithm_priority[i] == algorithm)
			return i;
	}
	return -1;
}

/**
  * gnutls_kx_get_name - Returns a string with the name of the specified key exchange algorithm
  * @algorithm: is a key exchange algorithm
  *
  * Returns a pointer to a string that contains the name 
  * of the specified key exchange algorithm.
  **/
const char *gnutls_kx_get_name(KXAlgorithm algorithm)
{
	char *ret = NULL;

	/* avoid prefix */
	GNUTLS_KX_ALG_LOOP(ret = p->name + sizeof("GNUTLS_KX_") - 1);

	return ret;
}

int _gnutls_kx_count()
{
	uint8 i, counter = 0;
	for (i = 0; i < MAX_KX; i++) {
		if (_gnutls_kx_is_ok(i) == 0)
			counter++;
	}
	return counter;
}


int _gnutls_kx_is_ok(KXAlgorithm algorithm)
{
	size_t ret = -1;
	GNUTLS_KX_ALG_LOOP(ret = p->algorithm);
	if (ret >= 0)
		ret = 0;
	else
		ret = 1;
	return ret;
}

/* Version */
int _gnutls_version_priority(GNUTLS_STATE state,
				     GNUTLS_Version version)
{				/* actually returns the priority */
	int i;

	if (state->gnutls_internals.ProtocolPriority.algorithm_priority==NULL) {
		gnutls_assert();
		return -1;
	}

	for (i = 0;
	     i <
	     state->gnutls_internals.ProtocolPriority.algorithms;
	     i++) {
		if (state->gnutls_internals.
		    ProtocolPriority.algorithm_priority[i] ==
		    version)
			return i;
	}
	return -1;
}

GNUTLS_Version _gnutls_version_lowest(GNUTLS_STATE state)
{				/* returns the lowest version supported */
	int i, min = 0xff;
	
	if (state->gnutls_internals.ProtocolPriority.algorithm_priority==NULL) {
		return GNUTLS_VERSION_UNKNOWN;
	} else
		for (i=0;i<state->gnutls_internals.ProtocolPriority.algorithms;i++) {
			if (state->gnutls_internals.ProtocolPriority.algorithm_priority[i] < min)
				min = state->gnutls_internals.ProtocolPriority.algorithm_priority[i];
		}

	if (min==0xff) return GNUTLS_VERSION_UNKNOWN; /* unknown version */

	return min;
}

GNUTLS_Version _gnutls_version_max(GNUTLS_STATE state)
{				/* returns the maximum version supported */
	int i, max=0x00;

	if (state->gnutls_internals.ProtocolPriority.algorithm_priority==NULL) {
		return GNUTLS_VERSION_UNKNOWN;
	} else
		for (i=0;i<state->gnutls_internals.ProtocolPriority.algorithms;i++) {
			if (state->gnutls_internals.ProtocolPriority.algorithm_priority[i] > max)
				max = state->gnutls_internals.ProtocolPriority.algorithm_priority[i];
		}
	
	if (max==0x00) return GNUTLS_VERSION_UNKNOWN; /* unknown version */
		
	return max;
}


/**
  * gnutls_protocol_get_name - Returns a string with the name of the specified SSL/TLS version
  * @version: is a (gnutls) version number
  *
  * Returns a string that contains the name 
  * of the specified TLS version.
  **/
const char *gnutls_protocol_get_name(GNUTLS_Version version)
{
	char *ret = NULL;

	/* avoid prefix */
	GNUTLS_VERSION_ALG_LOOP(ret =
			     p->name);
	return ret;
}

int _gnutls_version_get_minor(GNUTLS_Version version)
{
	int ret = -1;

	GNUTLS_VERSION_ALG_LOOP(ret = p->minor);
	return ret;
}

GNUTLS_Version _gnutls_version_get(int major, int minor)
{
	int ret = -1;

	GNUTLS_VERSION_LOOP(if ((p->major == major) && (p->minor == minor))
			    ret = p->id);
	return ret;
}

int _gnutls_version_get_major(GNUTLS_Version version)
{
	int ret = -1;

	GNUTLS_VERSION_ALG_LOOP(ret = p->major);
	return ret;
}

/* Version Functions */

int
_gnutls_version_is_supported(GNUTLS_STATE state,
			     const GNUTLS_Version version)
{
int ret;

	GNUTLS_VERSION_ALG_LOOP(ret = p->supported);
	if (ret == 0) return 0;

	if (_gnutls_version_priority( state, version) < 0)
		return 0; /* disabled by the user */
	else
		return 1;
}

/* Type to KX mappings */
KXAlgorithm _gnutls_map_kx_get_kx(CredType type)
{
	KXAlgorithm ret = -1;

	GNUTLS_KX_MAP_ALG_LOOP(ret = p->algorithm);
	return ret;
}

CredType _gnutls_map_kx_get_cred(KXAlgorithm algorithm)
{
	CredType ret = -1;
	GNUTLS_KX_MAP_LOOP(if (p->algorithm==algorithm) ret = p->type);

	return ret;
}


/* Cipher Suite's functions */
BulkCipherAlgorithm
_gnutls_cipher_suite_get_cipher_algo(const GNUTLS_CipherSuite suite)
{
	size_t ret = 0;
	GNUTLS_CIPHER_SUITE_ALG_LOOP(ret = p->block_algorithm);
	return ret;
}

KXAlgorithm _gnutls_cipher_suite_get_kx_algo(const GNUTLS_CipherSuite
					     suite)
{
	size_t ret = 0;

	GNUTLS_CIPHER_SUITE_ALG_LOOP(ret = p->kx_algorithm);
	return ret;

}

MACAlgorithm
_gnutls_cipher_suite_get_mac_algo(const GNUTLS_CipherSuite suite)
{				/* In bytes */
	size_t ret = 0;
	GNUTLS_CIPHER_SUITE_ALG_LOOP(ret = p->mac_algorithm);
	return ret;

}

const char *_gnutls_cipher_suite_get_name(GNUTLS_CipherSuite suite)
{
	char *ret = NULL;

	/* avoid prefix */
	GNUTLS_CIPHER_SUITE_ALG_LOOP(ret =
				     p->name + sizeof("GNUTLS_") -
					    1);

	return ret;
}


int _gnutls_cipher_suite_is_ok(GNUTLS_CipherSuite suite)
{
	size_t ret;
	char *name = NULL;

	GNUTLS_CIPHER_SUITE_ALG_LOOP(name = p->name);
	if (name != NULL)
		ret = 0;
	else
		ret = 1;
	return ret;

}

/* quite expensive */
int _gnutls_cipher_suite_count()
{
	GNUTLS_CipherSuite suite;
	int i, counter = 0, j;

	for (j = 0; j < MAX_CIPHERSUITE; j++) {
		suite.CipherSuite[0] = j;
		if (j != 0x00 && j != 0xF6)
			continue;

		for (i = 0; i < MAX_CIPHERSUITE; i++) {
			suite.CipherSuite[1] = i;
			if (_gnutls_cipher_suite_is_ok(suite) == 0)
				counter++;
		}

	}
	return counter;
}

#define SWAP(x, y) memcpy(tmp,x,size); \
		   memcpy(x,y,size); \
		   memcpy(y,tmp,size);

#define MAX_ELEM_SIZE 4
inline
    static int _gnutls_partition(GNUTLS_STATE state, void *_base,
				 size_t nmemb, size_t size,
				 int (*compar) (GNUTLS_STATE, const void *,
						const void *))
{
	uint8 *base = _base;
	uint8 tmp[MAX_ELEM_SIZE];
	uint8 ptmp[MAX_ELEM_SIZE];
	int pivot;
	int i, j;
	int full;

	i = pivot = 0;
	j = full = (nmemb - 1) * size;

	memcpy(ptmp, &base[0], size);	/* set pivot item */

	while (i < j) {
		while ((compar(state, &base[i], ptmp) <= 0) && (i < full)) {
			i += size;
		}
		while ((compar(state, &base[j], ptmp) >= 0) && (j > 0))
			j -= size;

		if (i < j) {
			SWAP(&base[j], &base[i]);
		}
	}

	if (j > pivot) {
		SWAP(&base[pivot], &base[j]);
		pivot = j;
	} else if (i < pivot) {
		SWAP(&base[pivot], &base[i]);
		pivot = i;
	}
	return pivot / size;
}

static void
_gnutls_qsort(GNUTLS_STATE state, void *_base, size_t nmemb, size_t size,
	      int (*compar) (GNUTLS_STATE, const void *, const void *))
{
	int pivot;
	char *base = _base;
	int snmemb = nmemb;

#ifdef DEBUG
	if (size > MAX_ELEM_SIZE) {
		gnutls_assert();
		_gnutls_log( "QSORT BUG\n");
		exit(1);
	}
#endif

	if (snmemb <= 1)
		return;
	pivot = _gnutls_partition(state, _base, nmemb, size, compar);

	_gnutls_qsort(state, base, pivot < nmemb ? pivot + 1 : pivot, size,
		      compar);
	_gnutls_qsort(state, &base[(pivot + 1) * size], nmemb - pivot - 1,
		      size, compar);
}


/* a compare function for KX algorithms (using priorities). For use with qsort */
static int
_gnutls_compare_algo(GNUTLS_STATE state, const void *i_A1,
		     const void *i_A2)
{
	KXAlgorithm kA1 =
	    _gnutls_cipher_suite_get_kx_algo(*(GNUTLS_CipherSuite *) i_A1);
	KXAlgorithm kA2 =
	    _gnutls_cipher_suite_get_kx_algo(*(GNUTLS_CipherSuite *) i_A2);
	BulkCipherAlgorithm cA1 =
	    _gnutls_cipher_suite_get_cipher_algo(*(GNUTLS_CipherSuite *)
						 i_A1);
	BulkCipherAlgorithm cA2 =
	    _gnutls_cipher_suite_get_cipher_algo(*(GNUTLS_CipherSuite *)
						 i_A2);
	MACAlgorithm mA1 =
	    _gnutls_cipher_suite_get_mac_algo(*(GNUTLS_CipherSuite *)
					      i_A1);
	MACAlgorithm mA2 =
	    _gnutls_cipher_suite_get_mac_algo(*(GNUTLS_CipherSuite *)
					      i_A2);

	int p1 = (_gnutls_kx_priority(state, kA1) + 1) * 100;
	int p2 = (_gnutls_kx_priority(state, kA2) + 1) * 100;
	p1 += (_gnutls_cipher_priority(state, cA1) + 1) * 10;
	p2 += (_gnutls_cipher_priority(state, cA2) + 1) * 10;
	p1 += _gnutls_mac_priority(state, mA1);
	p2 += _gnutls_mac_priority(state, mA2);

	if (p1 > p2) {
		return 1;
	} else {
		if (p1 == p2) {
			return 0;
		}
		return -1;
	}
}

#if 0
static void
_gnutls_bsort(GNUTLS_STATE state, void *_base, size_t nmemb,
	      size_t size, int (*compar) (GNUTLS_STATE, const void *,
					  const void *))
{
	int i, j;
	int full = nmemb * size;
	char *base = _base;
	char tmp[MAX_ELEM_SIZE];

	for (i = 0; i < full; i += size) {
		for (j = 0; j < full; j += size) {
			if (compar(state, &base[i], &base[j]) < 0) {
				SWAP(&base[j], &base[i]);
			}
		}
	}

}
#endif

int
_gnutls_supported_ciphersuites_sorted(GNUTLS_STATE state,
				      GNUTLS_CipherSuite ** ciphers)
{

	int i, ret_count, j = 0;
	int count = _gnutls_cipher_suite_count();
	GNUTLS_CipherSuite *tmp_ciphers;

	if (count == 0) {
		*ciphers = NULL;
		return 0;
	}

	tmp_ciphers = gnutls_malloc(count * sizeof(GNUTLS_CipherSuite));
	if (tmp_ciphers==NULL) return GNUTLS_E_MEMORY_ERROR;
	
	*ciphers = gnutls_malloc(count * sizeof(GNUTLS_CipherSuite));
	if (*ciphers==NULL) {
		gnutls_free(tmp_ciphers);
		return GNUTLS_E_MEMORY_ERROR;
	}


	for (i = 0; i < count; i++) {
		tmp_ciphers[i].CipherSuite[0] =
		    cs_algorithms[i].id.CipherSuite[0];
		tmp_ciphers[i].CipherSuite[1] =
		    cs_algorithms[i].id.CipherSuite[1];
	}

#ifdef SORT_DEBUG
	_gnutls_log( "Unsorted: \n");
	for (i = 0; i < count; i++)
		_gnutls_log( "\t%d: %s\n", i,
			_gnutls_cipher_suite_get_name((tmp_ciphers)[i]));
#endif

	_gnutls_qsort(state, tmp_ciphers, count,
		      sizeof(GNUTLS_CipherSuite), _gnutls_compare_algo);

	for (i = 0; i < count; i++) {
		if (_gnutls_kx_priority
		    (state,
		     _gnutls_cipher_suite_get_kx_algo(tmp_ciphers[i])) < 0)
			continue;
		if (_gnutls_mac_priority
		    (state,
		     _gnutls_cipher_suite_get_mac_algo(tmp_ciphers[i])) <
		    0)
			continue;
		if (_gnutls_cipher_priority
		    (state,
		     _gnutls_cipher_suite_get_cipher_algo(tmp_ciphers[i]))
		    < 0)
			continue;

		(*ciphers)[j].CipherSuite[0] =
		    tmp_ciphers[i].CipherSuite[0];
		(*ciphers)[j].CipherSuite[1] =
		    tmp_ciphers[i].CipherSuite[1];
		j++;
	}

#ifdef SORT_DEBUG
	_gnutls_log( "Sorted: \n");
	for (i = 0; i < j; i++)
		_gnutls_log( "\t%d: %s\n", i,
			_gnutls_cipher_suite_get_name((*ciphers)[i]));
	_gnutls_log( "SORT BUG\n");
	exit(0);
#endif

	ret_count = j;

	if (ret_count > 0 && ret_count != count) {
		*ciphers =
		    gnutls_realloc(*ciphers,
				   ret_count * sizeof(GNUTLS_CipherSuite));
	} else {
		if (ret_count != count) {
			gnutls_free(*ciphers);
			*ciphers = NULL;
		}
	}

	gnutls_free(tmp_ciphers);
	return ret_count;
}

int
_gnutls_supported_ciphersuites(GNUTLS_STATE state,
			       GNUTLS_CipherSuite ** _ciphers)
{

	int i, ret_count, j;
	int count = _gnutls_cipher_suite_count();
	GNUTLS_CipherSuite *tmp_ciphers;
	GNUTLS_CipherSuite* ciphers;

	*_ciphers = NULL;

	if (count == 0) {
		return 0;
	}

	tmp_ciphers = gnutls_malloc(count * sizeof(GNUTLS_CipherSuite));
	if ( tmp_ciphers==NULL)
		return GNUTLS_E_MEMORY_ERROR;

	ciphers = gnutls_malloc(count * sizeof(GNUTLS_CipherSuite));

	if ( ciphers==NULL) {
		gnutls_free( tmp_ciphers);
		return GNUTLS_E_MEMORY_ERROR;
	}
	
	for (i = 0; i < count; i++) {
		tmp_ciphers[i].CipherSuite[0] =
		    cs_algorithms[i].id.CipherSuite[0];
		tmp_ciphers[i].CipherSuite[1] =
		    cs_algorithms[i].id.CipherSuite[1];
	}

	for (i = j = 0; i < count; i++) {
		if (_gnutls_kx_priority
		    (state,
		     _gnutls_cipher_suite_get_kx_algo(tmp_ciphers[i])) < 0)
			continue;
		if (_gnutls_mac_priority
		    (state,
		     _gnutls_cipher_suite_get_mac_algo(tmp_ciphers[i])) <
		    0)
			continue;
		if (_gnutls_cipher_priority
		    (state,
		     _gnutls_cipher_suite_get_cipher_algo(tmp_ciphers[i]))
		    < 0)
			continue;

		ciphers[j].CipherSuite[0] = tmp_ciphers[i].CipherSuite[0];
		ciphers[j].CipherSuite[1] = tmp_ciphers[i].CipherSuite[1];
		j++;
	}

	ret_count = j;

	if (ret_count > 0 && ret_count != count) {
		ciphers =
		    gnutls_realloc(ciphers,
				   ret_count * sizeof(GNUTLS_CipherSuite));
	} else {
		if (ret_count != count) {
			gnutls_free(ciphers);
			ciphers = NULL;
		}
	}

	*_ciphers = ciphers;
	gnutls_free(tmp_ciphers);
	return ret_count;
}


/* For compression  */

/* returns the TLS numbers of the compression methods we support */
#define SUPPORTED_COMPRESSION_METHODS state->gnutls_internals.CompressionMethodPriority.algorithms
int
_gnutls_supported_compression_methods(GNUTLS_STATE state, uint8 ** comp)
{
	int i, tmp;

	*comp = gnutls_malloc(SUPPORTED_COMPRESSION_METHODS);
	if (*comp == NULL)
		return GNUTLS_E_MEMORY_ERROR;

	for (i = 0; i < SUPPORTED_COMPRESSION_METHODS; i++) {
		tmp = _gnutls_compression_get_num(state->gnutls_internals.
						  CompressionMethodPriority.
						  algorithm_priority[i]);
		if (tmp == -1) {
			gnutls_assert();
			/* we shouldn't get here */
			(*comp)[i] = 0;
			continue;
		}
		(*comp)[i] = (uint8) tmp;
	}

	return SUPPORTED_COMPRESSION_METHODS;
}
