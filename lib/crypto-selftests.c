/*
 * Copyright (C) 2013 Red Hat
 *
 * Author: Nikos Mavrogiannopoulos
 *
 * This file is part of GnuTLS.
 *
 * The GnuTLS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include "gnutls_int.h"
#include "errors.h"
#include <cipher_int.h>
#include <datum.h>
#include <gnutls/crypto.h>
#include <gnutls/self-test.h>
#include "errors.h"
#include <random.h>
#include <crypto.h>
#include <nettle/sha3.h>

#define STR(tag, tag_size, val) \
	.tag = (uint8_t*)val, \
	.tag_size = (sizeof(val)-1)

#define V(x) (x), (sizeof(x)/sizeof(x[0]))


/* This does check the AES and SHA implementation against test vectors.
 * This should not run under valgrind in order to use the native
 * cpu instructions (AES-NI or padlock).
 */

struct cipher_vectors_st {
	const uint8_t *key;
	unsigned int key_size;

	const uint8_t *plaintext;
	unsigned int plaintext_size;
	const uint8_t *ciphertext;	/* also of plaintext_size */

	const uint8_t *iv;
	unsigned int iv_size;
};

struct cipher_aead_vectors_st {
	const uint8_t *key;
	unsigned int key_size;

	const uint8_t *auth;
	unsigned int auth_size;

	const uint8_t *plaintext;
	unsigned int plaintext_size;
	const uint8_t *ciphertext;	/* also of plaintext_size */

	unsigned int iv_size;
	const uint8_t *iv;
	const uint8_t *tag;
	unsigned tag_size;
};

const struct cipher_aead_vectors_st chacha_poly1305_vectors[] = {
	{
	 STR(key, key_size,
	     "\x1c\x92\x40\xa5\xeb\x55\xd3\x8a\xf3\x33\x88\x86\x04\xf6\xb5\xf0\x47\x39\x17\xc1\x40\x2b\x80\x09\x9d\xca\x5c\xbc\x20\x70\x75\xc0"),
	 .auth = (void*)"\xf3\x33\x88\x86\x00\x00\x00\x00\x00\x00\x4e\x91",
	 .auth_size = 12,
	 .plaintext = (void*)"\x49\x6e\x74\x65\x72\x6e\x65\x74\x2d\x44\x72\x61\x66\x74\x73\x20\x61\x72\x65\x20\x64\x72\x61\x66\x74\x20\x64\x6f\x63\x75\x6d\x65\x6e\x74\x73\x20\x76\x61\x6c\x69\x64\x20\x66\x6f\x72\x20\x61\x20\x6d\x61\x78\x69\x6d\x75\x6d\x20\x6f\x66\x20\x73\x69\x78\x20\x6d\x6f\x6e\x74\x68\x73\x20\x61\x6e\x64\x20\x6d\x61\x79\x20\x62\x65\x20\x75\x70\x64\x61\x74\x65\x64\x2c\x20\x72\x65\x70\x6c\x61\x63\x65\x64\x2c\x20\x6f\x72\x20\x6f\x62\x73\x6f\x6c\x65\x74\x65\x64\x20\x62\x79\x20\x6f\x74\x68\x65\x72\x20\x64\x6f\x63\x75\x6d\x65\x6e\x74\x73\x20\x61\x74\x20\x61\x6e\x79\x20\x74\x69\x6d\x65\x2e\x20\x49\x74\x20\x69\x73\x20\x69\x6e\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x74\x6f\x20\x75\x73\x65\x20\x49\x6e\x74\x65\x72\x6e\x65\x74\x2d\x44\x72\x61\x66\x74\x73\x20\x61\x73\x20\x72\x65\x66\x65\x72\x65\x6e\x63\x65\x20\x6d\x61\x74\x65\x72\x69\x61\x6c\x20\x6f\x72\x20\x74\x6f\x20\x63\x69\x74\x65\x20\x74\x68\x65\x6d\x20\x6f\x74\x68\x65\x72\x20\x74\x68\x61\x6e\x20\x61\x73\x20\x2f\xe2\x80\x9c\x77\x6f\x72\x6b\x20\x69\x6e\x20\x70\x72\x6f\x67\x72\x65\x73\x73\x2e\x2f\xe2\x80\x9d",
	 .plaintext_size = 265,
	 .ciphertext = (void*)"\x64\xa0\x86\x15\x75\x86\x1a\xf4\x60\xf0\x62\xc7\x9b\xe6\x43\xbd\x5e\x80\x5c\xfd\x34\x5c\xf3\x89\xf1\x08\x67\x0a\xc7\x6c\x8c\xb2\x4c\x6c\xfc\x18\x75\x5d\x43\xee\xa0\x9e\xe9\x4e\x38\x2d\x26\xb0\xbd\xb7\xb7\x3c\x32\x1b\x01\x00\xd4\xf0\x3b\x7f\x35\x58\x94\xcf\x33\x2f\x83\x0e\x71\x0b\x97\xce\x98\xc8\xa8\x4a\xbd\x0b\x94\x81\x14\xad\x17\x6e\x00\x8d\x33\xbd\x60\xf9\x82\xb1\xff\x37\xc8\x55\x97\x97\xa0\x6e\xf4\xf0\xef\x61\xc1\x86\x32\x4e\x2b\x35\x06\x38\x36\x06\x90\x7b\x6a\x7c\x02\xb0\xf9\xf6\x15\x7b\x53\xc8\x67\xe4\xb9\x16\x6c\x76\x7b\x80\x4d\x46\xa5\x9b\x52\x16\xcd\xe7\xa4\xe9\x90\x40\xc5\xa4\x04\x33\x22\x5e\xe2\x82\xa1\xb0\xa0\x6c\x52\x3e\xaf\x45\x34\xd7\xf8\x3f\xa1\x15\x5b\x00\x47\x71\x8c\xbc\x54\x6a\x0d\x07\x2b\x04\xb3\x56\x4e\xea\x1b\x42\x22\x73\xf5\x48\x27\x1a\x0b\xb2\x31\x60\x53\xfa\x76\x99\x19\x55\xeb\xd6\x31\x59\x43\x4e\xce\xbb\x4e\x46\x6d\xae\x5a\x10\x73\xa6\x72\x76\x27\x09\x7a\x10\x49\xe6\x17\xd9\x1d\x36\x10\x94\xfa\x68\xf0\xff\x77\x98\x71\x30\x30\x5b\xea\xba\x2e\xda\x04\xdf\x99\x7b\x71\x4d\x6c\x6f\x2c\x29\xa6\xad\x5c\xb4\x02\x2b\x02\x70\x9b",
	 STR(iv, iv_size,
	     "\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08"),
	 .tag_size = 16,
	 .tag = (void *)
	 "\xee\xad\x9d\x67\x89\x0c\xbb\x22\x39\x23\x36\xfe\xa1\x85\x1f\x38"},
};

const struct cipher_aead_vectors_st aes128_gcm_vectors[] = {
	{
	 STR(key, key_size,
	     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
	 .auth = NULL,
	 .auth_size = 0,
	 .plaintext = NULL,
	 .plaintext_size = 0,
	 .ciphertext = NULL,
	 STR(iv, iv_size,
	     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
	 .tag_size = 16,
	 .tag = (void *)
	 "\x58\xe2\xfc\xce\xfa\x7e\x30\x61\x36\x7f\x1d\x57\xa4\xe7\x45\x5a"},
	{
	 STR(key, key_size,
	     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
	 .auth = NULL,
	 .auth_size = 0,
	 STR(plaintext, plaintext_size,
	     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
	 .ciphertext = (void *)
	 "\x03\x88\xda\xce\x60\xb6\xa3\x92\xf3\x28\xc2\xb9\x71\xb2\xfe\x78",
	 STR(iv, iv_size,
	     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
	 .tag_size = 16,
	 .tag = (void *)
	 "\xab\x6e\x47\xd4\x2c\xec\x13\xbd\xf5\x3a\x67\xb2\x12\x57\xbd\xdf"},
	{
	 STR(key, key_size,
	     "\xfe\xff\xe9\x92\x86\x65\x73\x1c\x6d\x6a\x8f\x94\x67\x30\x83\x08"),
	 .auth = (void *)
	 "\xfe\xed\xfa\xce\xde\xad\xbe\xef\xfe\xed\xfa\xce\xde\xad\xbe\xef\xab\xad\xda\xd2",
	 .auth_size = 20,
	 STR(plaintext, plaintext_size,
	     "\xd9\x31\x32\x25\xf8\x84\x06\xe5\xa5\x59\x09\xc5\xaf\xf5\x26\x9a\x86\xa7\xa9\x53\x15\x34\xf7\xda\x2e\x4c\x30\x3d\x8a\x31\x8a\x72\x1c\x3c\x0c\x95\x95\x68\x09\x53\x2f\xcf\x0e\x24\x49\xa6\xb5\x25\xb1\x6a\xed\xf5\xaa\x0d\xe6\x57\xba\x63\x7b\x39"),
	 .ciphertext = (void *)
	 "\x42\x83\x1e\xc2\x21\x77\x74\x24\x4b\x72\x21\xb7\x84\xd0\xd4\x9c\xe3\xaa\x21\x2f\x2c\x02\xa4\xe0\x35\xc1\x7e\x23\x29\xac\xa1\x2e\x21\xd5\x14\xb2\x54\x66\x93\x1c\x7d\x8f\x6a\x5a\xac\x84\xaa\x05\x1b\xa3\x0b\x39\x6a\x0a\xac\x97\x3d\x58\xe0\x91",
	 STR(iv, iv_size,
	     "\xca\xfe\xba\xbe\xfa\xce\xdb\xad\xde\xca\xf8\x88"),
	 .tag_size = 16,
	 .tag = (void *)
	 "\x5b\xc9\x4f\xbc\x32\x21\xa5\xdb\x94\xfa\xe9\x5a\xe7\x12\x1a\x47"}
};

const struct cipher_aead_vectors_st aes256_gcm_vectors[] = {
	{
	 STR(key, key_size,
	     "\xfe\xff\xe9\x92\x86\x65\x73\x1c\x6d\x6a\x8f\x94\x67\x30\x83\x08\xfe\xff\xe9\x92\x86\x65\x73\x1c\x6d\x6a\x8f\x94\x67\x30\x83\x08"),
	 .auth = NULL,
	 .auth_size = 0,
	 STR(plaintext, plaintext_size,
	     "\xd9\x31\x32\x25\xf8\x84\x06\xe5\xa5\x59\x09\xc5\xaf\xf5\x26\x9a\x86\xa7\xa9\x53\x15\x34\xf7\xda\x2e\x4c\x30\x3d\x8a\x31\x8a\x72\x1c\x3c\x0c\x95\x95\x68\x09\x53\x2f\xcf\x0e\x24\x49\xa6\xb5\x25\xb1\x6a\xed\xf5\xaa\x0d\xe6\x57\xba\x63\x7b\x39\x1a\xaf\xd2\x55"),
	 .ciphertext =
	 (uint8_t *)
	 "\x52\x2d\xc1\xf0\x99\x56\x7d\x07\xf4\x7f\x37\xa3\x2a\x84\x42\x7d\x64\x3a\x8c\xdc\xbf\xe5\xc0\xc9\x75\x98\xa2\xbd\x25\x55\xd1\xaa\x8c\xb0\x8e\x48\x59\x0d\xbb\x3d\xa7\xb0\x8b\x10\x56\x82\x88\x38\xc5\xf6\x1e\x63\x93\xba\x7a\x0a\xbc\xc9\xf6\x62\x89\x80\x15\xad",
	 STR(iv, iv_size,
	     "\xca\xfe\xba\xbe\xfa\xce\xdb\xad\xde\xca\xf8\x88"),
	 .tag_size = 16,
	 .tag =
	 (void *)
	 "\xb0\x94\xda\xc5\xd9\x34\x71\xbd\xec\x1a\x50\x22\x70\xe3\xcc\x6c"},

};

const struct cipher_aead_vectors_st aes256_ccm_vectors[] = {
	{ STR(key, key_size,
	     "\xfb\x76\x15\xb2\x3d\x80\x89\x1d\xd4\x70\x98\x0b\xc7\x95\x84\xc8\xb2\xfb\x64\xce\x60\x97\x8f\x4d\x17\xfc\xe4\x5a\x49\xe8\x30\xb7"),
	 .auth = NULL,
	 .auth_size = 0,
	 STR(plaintext, plaintext_size,
	     "\xa8\x45\x34\x8e\xc8\xc5\xb5\xf1\x26\xf5\x0e\x76\xfe\xfd\x1b\x1e"),
	 .ciphertext = (void *)
	     "\xcc\x88\x12\x61\xc6\xa7\xfa\x72\xb9\x6a\x17\x39\x17\x6b\x27\x7f",
	 STR(iv, iv_size,
	     "\xdb\xd1\xa3\x63\x60\x24\xb7\xb4\x02\xda\x7d\x6f"),
	 .tag_size = 16,
	 .tag = (void *)
	     "\x34\x72\xe1\x14\x5f\x2c\x0c\xbe\x14\x63\x49\x06\x2c\xf0\xe4\x23"},
	{
	 STR(key, key_size,
	     "\xfb\x76\x15\xb2\x3d\x80\x89\x1d\xd4\x70\x98\x0b\xc7\x95\x84\xc8\xb2\xfb\x64\xce\x60\x97\x87\x8d\x17\xfc\xe4\x5a\x49\xe8\x30\xb7"),
	 STR(auth, auth_size, "\x36"),
	 STR(plaintext, plaintext_size,
	     "\xa9"),
	 .ciphertext = (void *)
	     "\x9d",
	 STR(iv, iv_size,
	     "\xdb\xd1\xa3\x63\x60\x24\xb7\xb4\x02\xda\x7d\x6f"),
	 .tag_size = 16,
	 .tag = (void *)
	     "\x32\x61\xb1\xcf\x93\x14\x31\xe9\x9a\x32\x80\x67\x38\xec\xbd\x2a"},
};

const struct cipher_aead_vectors_st aes128_ccm_vectors[] = {
	{
	 STR(key, key_size,
	     "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF"),
	 STR(auth, auth_size, "\x08\xD0\x84\x21\x43\x01\x00\x00\x00\x00\x48\xDE\xAC\x02\x05\x00\x00\x00\x55\xCF\x00\x00\x51\x52\x53\x54"),
	 .plaintext = NULL,
	 .plaintext_size = 0,
	 STR(iv, iv_size,
	     "\xAC\xDE\x48\x00\x00\x00\x00\x01\x00\x00\x00\x05\x02"),
	 .tag_size = 8,
	 .tag = (void *)
	     "\x22\x3B\xC1\xEC\x84\x1A\xB5\x53"},
	{
	 STR(key, key_size,
	     "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f"),
	 STR(auth, auth_size, "\x00\x01\x02\x03\x04\x05\x06\x07"),
	 STR(plaintext, plaintext_size,
	     "\x20\x21\x22\x23"),
	 .ciphertext = (void *)
	     "\x71\x62\x01\x5b",
	 STR(iv, iv_size,
	     "\x10\x11\x12\x13\x14\x15\x16"),
	 .tag_size = 4,
	 .tag = (void *)
	     "\x4d\xac\x25\x5d"},
	/* from rfc3610 */
	{
	 STR(key, key_size,
	     "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF"),
	 STR(auth, auth_size, "\x00\x01\x02\x03\x04\x05\x06\x07"),
	 STR(plaintext, plaintext_size,
	     "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E"),
	 .ciphertext = (void *)
	     "\x01\x35\xD1\xB2\xC9\x5F\x41\xD5\xD1\xD4\xFE\xC1\x85\xD1\x66\xB8\x09\x4E\x99\x9D\xFE\xD9\x6C",
	 STR(iv, iv_size,
	     "\x00\x00\x00\x09\x08\x07\x06\xA0\xA1\xA2\xA3\xA4\xA5"),
	 .tag_size = 10,
	 .tag = (void *)
	     "\x04\x8C\x56\x60\x2C\x97\xAC\xBB\x74\x90"},
	{
	 STR(key, key_size,
	     "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF"),
	 STR(auth, auth_size, "\x00\x01\x02\x03\x04\x05\x06\x07"),
	 STR(plaintext, plaintext_size,
	     "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E"),
	 .ciphertext = (void *)
	     "\x58\x8C\x97\x9A\x61\xC6\x63\xD2\xF0\x66\xD0\xC2\xC0\xF9\x89\x80\x6D\x5F\x6B\x61\xDA\xC3\x84",
	 STR(iv, iv_size,
	     "\x00\x00\x00\x03\x02\x01\x00\xA0\xA1\xA2\xA3\xA4\xA5"),
	 .tag_size = 8,
	 .tag = (void *)
	     "\x17\xE8\xD1\x2C\xFD\xF9\x26\xE0"},
};

const struct cipher_vectors_st aes128_cbc_vectors[] = {
	{
	 STR(key, key_size,
	     "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c"),
	 STR(plaintext, plaintext_size,
	     "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"),
	 .ciphertext = (uint8_t *)
	 "\x76\x49\xab\xac\x81\x19\xb2\x46\xce\xe9\x8e\x9b\x12\xe9\x19\x7d",
	 STR(iv, iv_size,
	     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"),
	 },
	{
	 STR(key, key_size,
	     "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c"),
	 STR(plaintext, plaintext_size,
	     "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"),
	 .ciphertext =
	 (uint8_t *)
	 "\x50\x86\xcb\x9b\x50\x72\x19\xee\x95\xdb\x11\x3a\x91\x76\x78\xb2",
	 STR(iv, iv_size,
	     "\x76\x49\xab\xac\x81\x19\xb2\x46\xce\xe9\x8e\x9b\x12\xe9\x19\x7d"),
	 },
};

const struct cipher_vectors_st aes192_cbc_vectors[] = {
	{
	 STR(key, key_size,
	     "\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5\x62\xf8\xea\xd2\x52\x2c\x6b\x7b"),
	 STR(plaintext, plaintext_size,
	     "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"),
	 .ciphertext = (uint8_t *)
	 "\x4f\x02\x1d\xb2\x43\xbc\x63\x3d\x71\x78\x18\x3a\x9f\xa0\x71\xe8",
	 STR(iv, iv_size,
	     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"),
	 },
	{
	 STR(key, key_size,
	     "\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5\x62\xf8\xea\xd2\x52\x2c\x6b\x7b"),
	 STR(plaintext, plaintext_size,
	     "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"),
	 .ciphertext = (uint8_t *)
	 "\xb4\xd9\xad\xa9\xad\x7d\xed\xf4\xe5\xe7\x38\x76\x3f\x69\x14\x5a",
	 STR(iv, iv_size,
	     "\x4F\x02\x1D\xB2\x43\xBC\x63\x3D\x71\x78\x18\x3A\x9F\xA0\x71\xE8"),
	 },
};

const struct cipher_vectors_st aes256_cbc_vectors[] = {
	{
	 STR(key, key_size,
	     "\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4"),
	 STR(plaintext, plaintext_size,
	     "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"),
	 .ciphertext = (uint8_t *)
	 "\xF5\x8C\x4C\x04\xD6\xE5\xF1\xBA\x77\x9E\xAB\xFB\x5F\x7B\xFB\xD6",
	 STR(iv, iv_size,
	     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"),
	 },
	{
	 STR(key, key_size,
	     "\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4"),
	 STR(plaintext, plaintext_size,
	     "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"),
	 .ciphertext = (uint8_t *)
	 "\x9c\xfc\x4e\x96\x7e\xdb\x80\x8d\x67\x9f\x77\x7b\xc6\x70\x2c\x7d",
	 STR(iv, iv_size,
	     "\xF5\x8C\x4C\x04\xD6\xE5\xF1\xBA\x77\x9E\xAB\xFB\x5F\x7B\xFB\xD6"),
	 },
};

const struct cipher_vectors_st tdes_cbc_vectors[] = {
/* First 2 from https://www.cosic.esat.kuleuven.be/nessie/testvectors/bc/des/Triple-Des-3-Key-192-64.unverified.test-vectors */
	{
	 STR(key, key_size,
	     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17"),
	 STR(plaintext, plaintext_size,
	     "\x98\x26\x62\x60\x55\x53\x24\x4D"),
	 .ciphertext = (uint8_t *)
	 "\x00\x11\x22\x33\x44\x55\x66\x77",
	 STR(iv, iv_size, "\x00\x00\x00\x00\x00\x00\x00\x00"),
	 },
	{
	 STR(key, key_size,
	     "\x2B\xD6\x45\x9F\x82\xC5\xB3\x00\x95\x2C\x49\x10\x48\x81\xFF\x48\x2B\xD6\x45\x9F\x82\xC5\xB3\x00"),
	 STR(plaintext, plaintext_size,
	     "\x85\x98\x53\x8A\x8E\xCF\x11\x7D"),
	 .ciphertext = (uint8_t *)
	 "\xEA\x02\x47\x14\xAD\x5C\x4D\x84",
	 STR(iv, iv_size, "\x00\x00\x00\x00\x00\x00\x00\x00"),
	 },
};

const struct cipher_vectors_st arcfour_vectors[] = { /* RFC6229 */
	{
	 STR(key, key_size,
	     "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18"),
	 STR(plaintext, plaintext_size,
	     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
	 .ciphertext = (uint8_t *)
	     "\x05\x95\xe5\x7f\xe5\xf0\xbb\x3c\x70\x6e\xda\xc8\xa4\xb2\xdb\x11",
	 .iv = NULL,
	 .iv_size = 0
	},
	{
	 STR(key, key_size,
	     "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20"),
	 STR(plaintext, plaintext_size,
	     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
	 .ciphertext = (uint8_t *)
	     "\xea\xa6\xbd\x25\x88\x0b\xf9\x3d\x3f\x5d\x1e\x4c\xa2\x61\x1d\x91",
	 .iv = NULL,
	 .iv_size = 0
	},
	{
	 STR(key, key_size,
	     "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"),
	 STR(plaintext, plaintext_size,
	     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
	 .ciphertext = (uint8_t *)
	 "\x9a\xc7\xcc\x9a\x60\x9d\x1e\xf7\xb2\x93\x28\x99\xcd\xe4\x1b\x97",
	 .iv = NULL,
	 .iv_size = 0
	},
};

static int test_cipher(gnutls_cipher_algorithm_t cipher,
		       const struct cipher_vectors_st *vectors,
		       size_t vectors_size)
{
	gnutls_cipher_hd_t hd;
	int ret;
	unsigned int i;
	uint8_t tmp[384];
	gnutls_datum_t key, iv = {NULL, 0};

	for (i = 0; i < vectors_size; i++) {
		key.data = (void *) vectors[i].key;
		key.size = vectors[i].key_size;

		if (vectors[i].iv != NULL) {
			iv.data = (void *) vectors[i].iv;
			iv.size = gnutls_cipher_get_iv_size(cipher);
		}

		if (iv.size != vectors[i].iv_size)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		ret = gnutls_cipher_init(&hd, cipher, &key, &iv);
		if (ret < 0) {
			_gnutls_debug_log("error initializing: %s\n",
					  gnutls_cipher_get_name(cipher));
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}

		ret =
		    gnutls_cipher_encrypt2(hd,
					   vectors[i].plaintext,
					   vectors[i].plaintext_size,
					   tmp, sizeof(tmp));
		if (ret < 0)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		gnutls_cipher_deinit(hd);

		if (memcmp
		    (tmp, vectors[i].ciphertext,
		     vectors[i].plaintext_size) != 0) {
			_gnutls_debug_log("%s test vector %d failed!\n",
					  gnutls_cipher_get_name(cipher),
					  i);
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}
	}

	iv.size = gnutls_cipher_get_iv_size(cipher);

	for (i = 0; i < vectors_size; i++) {
		key.data = (void *) vectors[i].key;
		key.size = vectors[i].key_size;

		iv.data = (void *) vectors[i].iv;

		ret = gnutls_cipher_init(&hd, cipher, &key, &iv);
		if (ret < 0)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		ret =
		    gnutls_cipher_decrypt2(hd,
					   vectors[i].ciphertext, 
					   vectors[i].plaintext_size, tmp,
					   sizeof(tmp));
		if (ret < 0) {
			_gnutls_debug_log
			    ("%s decryption of test vector %d failed!\n",
			     gnutls_cipher_get_name(cipher), i);
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}

		gnutls_cipher_deinit(hd);

		if (memcmp
		    (tmp, vectors[i].plaintext,
		     vectors[i].plaintext_size) != 0) {
			_gnutls_debug_log("%s test vector %d failed!\n",
					  gnutls_cipher_get_name(cipher),
					  i);
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}
	}

	_gnutls_debug_log
	    ("%s self check succeeded\n",
	     gnutls_cipher_get_name(cipher));

	return 0;
}

/* AEAD modes */
static int test_cipher_aead(gnutls_cipher_algorithm_t cipher,
			    const struct cipher_aead_vectors_st *vectors,
			    size_t vectors_size)
{
	gnutls_aead_cipher_hd_t hd;
	int ret;
	unsigned int i;
	uint8_t tmp[384];
	uint8_t tmp2[384];
	gnutls_datum_t key, iv;
	size_t s, s2;
	unsigned tag_size;

	for (i = 0; i < vectors_size; i++) {
		memset(tmp, 0, sizeof(tmp));
		key.data = (void *) vectors[i].key;
		key.size = vectors[i].key_size;

		iv.data = (void *) vectors[i].iv;
		iv.size = vectors[i].iv_size;
		tag_size = vectors[i].tag_size;

		if (tag_size > gnutls_cipher_get_tag_size(cipher)) {
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}
#if 0
		if (iv.size != vectors[i].iv_size)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
#endif
		ret = gnutls_aead_cipher_init(&hd, cipher, &key);
		if (ret < 0) {
			_gnutls_debug_log("error initializing: %s\n",
					  gnutls_cipher_get_name(cipher));
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}
		_gnutls_debug_log("initialized: %s\n",
					  gnutls_cipher_get_name(cipher));

		s = sizeof(tmp);

		ret =
		    gnutls_aead_cipher_encrypt(hd,
					   iv.data, iv.size,
					   vectors[i].auth, vectors[i].auth_size,
					   vectors[i].tag_size,
					   vectors[i].plaintext,
					   vectors[i].plaintext_size,
					   tmp, &s);
		if (ret < 0)
			return
			    gnutls_assert_val
			    (GNUTLS_E_SELF_TEST_ERROR);

		if (s != vectors[i].plaintext_size + tag_size) {
			return
			    gnutls_assert_val
			    (GNUTLS_E_SELF_TEST_ERROR);
		}

		if (memcmp(tmp+vectors[i].plaintext_size, vectors[i].tag, tag_size) != 0) {
			_gnutls_debug_log
			    ("%s test vector %d failed (tag)!\n",
			     gnutls_cipher_get_name(cipher), i);
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}

		if (vectors[i].plaintext_size > 0) {
			
			if (memcmp
			    (tmp, vectors[i].ciphertext,
			     vectors[i].plaintext_size) != 0) {
				_gnutls_debug_log
				    ("%s test vector %d failed!\n",
				     gnutls_cipher_get_name(cipher), i);

				return
				    gnutls_assert_val
				    (GNUTLS_E_SELF_TEST_ERROR);
			}
		}

		/* check decryption */
		{
			s2 = sizeof(tmp2);
			ret =
			    gnutls_aead_cipher_decrypt(hd,
						   iv.data, iv.size,
						   vectors[i].auth, vectors[i].auth_size,
						   vectors[i].tag_size,
						   tmp, s,
						   tmp2, &s2);
			if (ret < 0)
				return
				    gnutls_assert_val
				    (GNUTLS_E_SELF_TEST_ERROR);

			if (s2 != vectors[i].plaintext_size && memcmp(tmp, vectors[i].plaintext, vectors[i].plaintext_size) != 0) {
				_gnutls_debug_log("%s test vector %d failed (decryption)!\n",
					gnutls_cipher_get_name(cipher), i);
				return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
			}

			/* test tag verification */
			if (s > 0) {
				tmp[0]++;

				s2 = sizeof(tmp2);
				ret =
				    gnutls_aead_cipher_decrypt(hd,
							   iv.data, iv.size,
							   vectors[i].auth, vectors[i].auth_size,
							   vectors[i].tag_size,
							   tmp, s,
							   tmp2, &s2);

				if (ret >= 0) {
					_gnutls_debug_log("%s: tag check failed\n", gnutls_cipher_get_name(cipher));
					return
					    gnutls_assert_val
					    (GNUTLS_E_SELF_TEST_ERROR);
				}
			}
		}

		gnutls_aead_cipher_deinit(hd);
	}

	_gnutls_debug_log
	    ("%s self check succeeded\n",
	     gnutls_cipher_get_name(cipher));

	return 0;

}

struct hash_vectors_st {
	const uint8_t *plaintext;
	unsigned int plaintext_size;
	const uint8_t *output;
	unsigned int output_size;
};

const struct hash_vectors_st md5_vectors[] = {
	{
	 STR(plaintext, plaintext_size, "abcdefghijklmnopqrstuvwxyz"),
	 STR(output, output_size,
	     "\xc3\xfc\xd3\xd7\x61\x92\xe4\x00\x7d\xfb\x49\x6c\xca\x67\xe1\x3b"),
	 },
};

const struct hash_vectors_st sha1_vectors[] = {
	{
	 STR(plaintext, plaintext_size, "what do ya want for nothing?"),
	 STR(output, output_size,
	     "\x8f\x82\x03\x94\xf9\x53\x35\x18\x20\x45\xda\x24\xf3\x4d\xe5\x2b\xf8\xbc\x34\x32"),
	 },
	{
	 STR(plaintext, plaintext_size,
	     "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopqabcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopqabcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
	 STR(output, output_size,
	     "\xbe\xae\xd1\x6d\x65\x8e\xc7\x92\x9e\xdf\xd6\x2b\xfa\xfe\xac\x29\x9f\x0d\x74\x4d"),
	 },
};

const struct hash_vectors_st sha224_vectors[] = {
	{
	 STR(plaintext, plaintext_size,
	     "The quick brown fox jumps over the lazy dog"),
	 STR(output, output_size,
	     "\x73\x0e\x10\x9b\xd7\xa8\xa3\x2b\x1c\xb9\xd9\xa0\x9a\xa2\x32\x5d\x24\x30\x58\x7d\xdb\xc0\xc3\x8b\xad\x91\x15\x25"),
	 },
};

const struct hash_vectors_st sha256_vectors[] = {
	{
	 STR(plaintext, plaintext_size,
	     "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
	 STR(output, output_size,
	     "\x24\x8d\x6a\x61\xd2\x06\x38\xb8\xe5\xc0\x26\x93\x0c\x3e\x60\x39\xa3\x3c\xe4\x59\x64\xff\x21\x67\xf6\xec\xed\xd4\x19\xdb\x06\xc1"),
	 },
	{
	 STR(plaintext, plaintext_size,
	     "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopqabcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopqabcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
	 STR(output, output_size,
	     "\x50\xea\x82\x5d\x96\x84\xf4\x22\x9c\xa2\x9f\x1f\xec\x51\x15\x93\xe2\x81\xe4\x6a\x14\x0d\x81\xe0\x00\x5f\x8f\x68\x86\x69\xa0\x6c"),
	 },
};

const struct hash_vectors_st sha384_vectors[] = {
	{
	 STR(plaintext, plaintext_size,
	     "The quick brown fox jumps over the lazy dog"),
	 STR(output, output_size,
	     "\xca\x73\x7f\x10\x14\xa4\x8f\x4c\x0b\x6d\xd4\x3c\xb1\x77\xb0\xaf\xd9\xe5\x16\x93\x67\x54\x4c\x49\x40\x11\xe3\x31\x7d\xbf\x9a\x50\x9c\xb1\xe5\xdc\x1e\x85\xa9\x41\xbb\xee\x3d\x7f\x2a\xfb\xc9\xb1"),
	 },
};

const struct hash_vectors_st sha512_vectors[] = {
	{
	 STR(plaintext, plaintext_size,
	     "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"),
	 STR(output, output_size,
	     "\x8e\x95\x9b\x75\xda\xe3\x13\xda\x8c\xf4\xf7\x28\x14\xfc\x14\x3f\x8f\x77\x79\xc6\xeb\x9f\x7f\xa1\x72\x99\xae\xad\xb6\x88\x90\x18\x50\x1d\x28\x9e\x49\x00\xf7\xe4\x33\x1b\x99\xde\xc4\xb5\x43\x3a\xc7\xd3\x29\xee\xb6\xdd\x26\x54\x5e\x96\xe5\x5b\x87\x4b\xe9\x09"),
	 },
};

const struct hash_vectors_st sha3_224_vectors[] = {
	{
	 STR(plaintext, plaintext_size,
	     "\xC1\xEC\xFD\xFC"),
	 STR(output, output_size,
	     "\xA3\x3C\x58\xDF\x8A\x80\x26\xF0\xF9\x59\x19\x66\xBD\x6D\x00\xEE\xD3\xB1\xE8\x29\x58\x0A\xB9\xBE\x26\x8C\xAF\x39"),
	 },
};

const struct hash_vectors_st sha3_256_vectors[] = {
	{
	 STR(plaintext, plaintext_size,
	     "\xC1\xEC\xFD\xFC"),
	 STR(output, output_size,
	     "\xC5\x85\x9B\xE8\x25\x60\xCC\x87\x89\x13\x3F\x7C\x83\x4A\x6E\xE6\x28\xE3\x51\xE5\x04\xE6\x01\xE8\x05\x9A\x06\x67\xFF\x62\xC1\x24"),
	 }
};

const struct hash_vectors_st sha3_384_vectors[] = {
	{
	 STR(plaintext, plaintext_size,
	     "\x4A\x4F\x20\x24\x84\x51\x25\x26"),
	 STR(output, output_size,
	     "\x89\xDB\xF4\xC3\x9B\x8F\xB4\x6F\xDF\x0A\x69\x26\xCE\xC0\x35\x5A\x4B\xDB\xF9\xC6\xA4\x46\xE1\x40\xB7\xC8\xBD\x08\xFF\x6F\x48\x9F\x20\x5D\xAF\x8E\xFF\xE1\x60\xF4\x37\xF6\x74\x91\xEF\x89\x7C\x23"),
	 },
};

const struct hash_vectors_st sha3_512_vectors[] = {
	{
	 STR(plaintext, plaintext_size,
	     "\x82\xE1\x92\xE4\x04\x3D\xDC\xD1\x2E\xCF\x52\x96\x9D\x0F\x80\x7E\xED"),
	 STR(output, output_size,
	     "\x96\x44\xE3\xC9\x0B\x67\xE2\x21\x24\xE9\x6D\xFE\xDC\xE5\x3D\x33\xC4\x60\xF1\x32\x86\x8F\x09\x75\xD1\x8B\x22\xCF\xD5\x9F\x63\x7D\xD8\x5A\xA4\x05\xE3\x98\x08\xA4\x55\x70\xA4\x98\xC0\xB8\xF2\xCB\xA5\x9F\x8E\x14\x37\xEA\xEF\x89\xF2\x0B\x88\x29\x8A\xDF\xA2\xDE"),
	 },
};

#define HASH_DATA_SIZE 64

/* SHA1 and other hashes */
static int test_digest(gnutls_digest_algorithm_t dig,
		       const struct hash_vectors_st *vectors,
		       size_t vectors_size)
{
	uint8_t data[HASH_DATA_SIZE];
	unsigned int i;
	int ret;
	size_t data_size;
	gnutls_hash_hd_t hd;

	if (_gnutls_digest_exists(dig) == 0)
		return 0;

	for (i = 0; i < vectors_size; i++) {
		ret = gnutls_hash_init(&hd, dig);
		if (ret < 0) {
			_gnutls_debug_log("error initializing: %s\n",
					  gnutls_digest_get_name(dig));
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}

		ret = gnutls_hash(hd, vectors[i].plaintext, 1);
		if (ret < 0)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		ret = gnutls_hash(hd,
				  &vectors[i].plaintext[1],
				  vectors[i].plaintext_size - 1);
		if (ret < 0)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		gnutls_hash_deinit(hd, data);

		data_size = gnutls_hash_get_len(dig);
		if (ret < 0)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		if (data_size != vectors[i].output_size ||
		    memcmp(data, vectors[i].output,
			   vectors[i].output_size) != 0) {
			_gnutls_debug_log("%s test vector %d failed!\n",
					  gnutls_digest_get_name(dig), i);
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}
	}

	_gnutls_debug_log("%s self check succeeded\n",
			  gnutls_digest_get_name(dig));

	return 0;
}


struct mac_vectors_st {
	const uint8_t *key;
	unsigned int key_size;
	const uint8_t *plaintext;
	unsigned int plaintext_size;
	const uint8_t *output;
	unsigned int output_size;
};

const struct mac_vectors_st hmac_md5_vectors[] = {
	{
	 STR(key, key_size, "Jefe"),
	 STR(plaintext, plaintext_size, "what do ya want for nothing?"),
	 STR(output, output_size,
	     "\x75\x0c\x78\x3e\x6a\xb0\xb5\x03\xea\xa8\x6e\x31\x0a\x5d\xb7\x38"),
	 },
};

const struct mac_vectors_st hmac_sha1_vectors[] = {
	{
	 STR(key, key_size,
	     "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"),
	 STR(plaintext, plaintext_size, "Hi There"),
	 STR(output, output_size,
	     "\x67\x5b\x0b\x3a\x1b\x4d\xdf\x4e\x12\x48\x72\xda\x6c\x2f\x63\x2b\xfe\xd9\x57\xe9"),
	 },
};

	    /* from rfc4231 */
const struct mac_vectors_st hmac_sha224_vectors[] = {
	{
	 STR(key, key_size,
	     "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"),
	 STR(plaintext, plaintext_size, "Hi There"),
	 STR(output, output_size,
	     "\x89\x6f\xb1\x12\x8a\xbb\xdf\x19\x68\x32\x10\x7c\xd4\x9d\xf3\x3f\x47\xb4\xb1\x16\x99\x12\xba\x4f\x53\x68\x4b\x22"),
	 },
};

const struct mac_vectors_st hmac_sha256_vectors[] = {
	{
	 STR(key, key_size,
	     "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"),
	 STR(plaintext, plaintext_size, "Hi There"),
	 STR(output, output_size,
	     "\xb0\x34\x4c\x61\xd8\xdb\x38\x53\x5c\xa8\xaf\xce\xaf\x0b\xf1\x2b\x88\x1d\xc2\x00\xc9\x83\x3d\xa7\x26\xe9\x37\x6c\x2e\x32\xcf\xf7"),
	 },
};

const struct mac_vectors_st hmac_sha384_vectors[] = {
	{
	 STR(key, key_size,
	     "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"),
	 STR(plaintext, plaintext_size, "Hi There"),
	 STR(output, output_size,
	     "\xaf\xd0\x39\x44\xd8\x48\x95\x62\x6b\x08\x25\xf4\xab\x46\x90\x7f\x15\xf9\xda\xdb\xe4\x10\x1e\xc6\x82\xaa\x03\x4c\x7c\xeb\xc5\x9c\xfa\xea\x9e\xa9\x07\x6e\xde\x7f\x4a\xf1\x52\xe8\xb2\xfa\x9c\xb6"),
	 },
};

const struct mac_vectors_st hmac_sha512_vectors[] = {
	{
	 STR(key, key_size,
	     "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"),
	 STR(plaintext, plaintext_size, "Hi There"),
	 STR(output, output_size,
	     "\x87\xaa\x7c\xde\xa5\xef\x61\x9d\x4f\xf0\xb4\x24\x1a\x1d\x6c\xb0\x23\x79\xf4\xe2\xce\x4e\xc2\x78\x7a\xd0\xb3\x05\x45\xe1\x7c\xde\xda\xa8\x33\xb7\xd6\xb8\xa7\x02\x03\x8b\x27\x4e\xae\xa3\xf4\xe4\xbe\x9d\x91\x4e\xeb\x61\xf1\x70\x2e\x69\x6c\x20\x3a\x12\x68\x54"),
	 },
};

static int test_mac(gnutls_mac_algorithm_t mac,
		    const struct mac_vectors_st *vectors,
		    size_t vectors_size)
{
	uint8_t data[HASH_DATA_SIZE];
	unsigned int i;
	int ret;
	size_t data_size;
	gnutls_hmac_hd_t hd;

	for (i = 0; i < vectors_size; i++) {
		ret = gnutls_hmac_init(&hd,
				       mac, vectors[i].key,
				       vectors[i].key_size);

		if (ret < 0) {
			_gnutls_debug_log("error initializing: MAC-%s\n",
					  gnutls_mac_get_name(mac));
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}

		ret = gnutls_hmac(hd, vectors[i].plaintext, 1);
		if (ret < 0)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		ret = gnutls_hmac(hd,
				  &vectors[i].plaintext[1],
				  vectors[i].plaintext_size - 1);
		if (ret < 0)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		gnutls_hmac_deinit(hd, data);

		data_size = gnutls_hmac_get_len(mac);
		if (ret < 0)
			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);

		if (data_size != vectors[i].output_size ||
		    memcmp(data, vectors[i].output,
			   vectors[i].output_size) != 0) {

			_gnutls_debug_log
			    ("MAC-%s test vector %d failed!\n",
			     gnutls_mac_get_name(mac), i);

			return gnutls_assert_val(GNUTLS_E_SELF_TEST_ERROR);
		}
	}

	_gnutls_debug_log
	    ("MAC-%s self check succeeded\n",
	     gnutls_mac_get_name(mac));

	return 0;
}

#define CASE(x, func, vectors) case x: \
			ret = func(x, V(vectors)); \
			if (all == 0 || ret < 0) \
				return ret

#define NON_FIPS_CASE(x, func, vectors) case x: \
			if (_gnutls_fips_mode_enabled() == 0) { \
				ret = func(x, V(vectors)); \
				if (all == 0 || ret < 0) \
					return ret; \
			}

#define FIPS_STARTUP_ONLY_TEST_CASE(x, func, vectors) case x: \
			if (_gnutls_fips_mode_enabled() != 1) { \
				ret = func(x, V(vectors)); \
				if (all == 0 || ret < 0) \
					return ret; \
			}

/*-
 * gnutls_cipher_self_test:
 * @all: if non-zero then tests to all ciphers are performed.
 * @cipher: the encryption algorithm to use
 *
 * This function will run self tests on the provided cipher or all
 * available ciphers if @all is non-zero.
 *
 * Returns: Zero or a negative error code on error.
 *
 * Since: 3.3.0-FIPS140
 -*/
int gnutls_cipher_self_test(unsigned all, gnutls_cipher_algorithm_t cipher)
{
	int ret;

	if (all != 0)
		cipher = GNUTLS_CIPHER_UNKNOWN;

	switch (cipher) {
	case GNUTLS_CIPHER_UNKNOWN:
		CASE(GNUTLS_CIPHER_AES_128_CCM, test_cipher_aead,
		     aes128_ccm_vectors);
		CASE(GNUTLS_CIPHER_AES_256_CCM, test_cipher_aead,
		     aes256_ccm_vectors);
		CASE(GNUTLS_CIPHER_AES_128_CBC, test_cipher,
		     aes128_cbc_vectors);
		CASE(GNUTLS_CIPHER_AES_192_CBC, test_cipher,
		     aes192_cbc_vectors);
		CASE(GNUTLS_CIPHER_AES_256_CBC, test_cipher,
		     aes256_cbc_vectors);
		CASE(GNUTLS_CIPHER_3DES_CBC, test_cipher,
		     tdes_cbc_vectors);
		NON_FIPS_CASE(GNUTLS_CIPHER_ARCFOUR_128, test_cipher,
		     arcfour_vectors);
		CASE(GNUTLS_CIPHER_AES_128_GCM, test_cipher_aead,
		     aes128_gcm_vectors);
		CASE(GNUTLS_CIPHER_AES_256_GCM, test_cipher_aead,
		     aes256_gcm_vectors);
		NON_FIPS_CASE(GNUTLS_CIPHER_CHACHA20_POLY1305, test_cipher_aead,
		     chacha_poly1305_vectors);
		break;
	default:
		return gnutls_assert_val(GNUTLS_E_NO_SELF_TEST);
	}

	return 0;
}

/*-
 * gnutls_mac_self_test:
 * @all: if non-zero then tests to all ciphers are performed.
 * @mac: the message authentication algorithm to use
 *
 * This function will run self tests on the provided mac.
 *
 * Returns: Zero or a negative error code on error.
 *
 * Since: 3.3.0-FIPS140
 -*/
int gnutls_mac_self_test(unsigned all, gnutls_mac_algorithm_t mac)
{
	int ret;

	if (all != 0)
		mac = GNUTLS_MAC_UNKNOWN;

	switch (mac) {
	case GNUTLS_MAC_UNKNOWN:
		FIPS_STARTUP_ONLY_TEST_CASE(GNUTLS_MAC_MD5, test_mac, hmac_md5_vectors);
		CASE(GNUTLS_MAC_SHA1, test_mac, hmac_sha1_vectors);
		CASE(GNUTLS_MAC_SHA224, test_mac, hmac_sha224_vectors);
		CASE(GNUTLS_MAC_SHA256, test_mac, hmac_sha256_vectors);
		CASE(GNUTLS_MAC_SHA384, test_mac, hmac_sha384_vectors);
		CASE(GNUTLS_MAC_SHA512, test_mac, hmac_sha512_vectors);

		break;
	default:
		return gnutls_assert_val(GNUTLS_E_NO_SELF_TEST);
	}

	return 0;
}

/*-
 * gnutls_digest_self_test:
 * @all: if non-zero then tests to all ciphers are performed.
 * @digest: the digest algorithm to use
 *
 * This function will run self tests on the provided digest.
 *
 * Returns: Zero or a negative error code on error.
 *
 * Since: 3.3.0-FIPS140
 -*/
int gnutls_digest_self_test(unsigned all, gnutls_digest_algorithm_t digest)
{
	int ret;

	if (all != 0)
		digest = GNUTLS_DIG_UNKNOWN;

	switch (digest) {
	case GNUTLS_DIG_UNKNOWN:
		FIPS_STARTUP_ONLY_TEST_CASE(GNUTLS_DIG_MD5, test_digest, md5_vectors);
		CASE(GNUTLS_DIG_SHA1, test_digest, sha1_vectors);
		CASE(GNUTLS_DIG_SHA224, test_digest, sha224_vectors);
		CASE(GNUTLS_DIG_SHA256, test_digest, sha256_vectors);
		CASE(GNUTLS_DIG_SHA384, test_digest, sha384_vectors);
		CASE(GNUTLS_DIG_SHA512, test_digest, sha512_vectors);
#ifdef NETTLE_SHA3_FIPS202
		CASE(GNUTLS_DIG_SHA3_224, test_digest, sha3_224_vectors);
		CASE(GNUTLS_DIG_SHA3_256, test_digest, sha3_256_vectors);
		CASE(GNUTLS_DIG_SHA3_384, test_digest, sha3_384_vectors);
		CASE(GNUTLS_DIG_SHA3_512, test_digest, sha3_512_vectors);
#endif
		break;
	default:
		return gnutls_assert_val(GNUTLS_E_NO_SELF_TEST);
	}

	return 0;
}
