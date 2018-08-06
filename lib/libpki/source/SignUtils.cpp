#include <pki/SignUtils.h>

pki::sign::SignatureAlgo pki::sign::getSignatureAlgo(pki::sign::SignatureId sign_id)
{
	SignatureAlgo sign_algo = SIGN_ALGO_RSA4096;

	switch (sign_id)
	{
	case (pki::sign::SIGN_ID_RSA4096_SHA1):
	case (pki::sign::SIGN_ID_RSA4096_SHA256):
		sign_algo = SIGN_ALGO_RSA4096;
		break;
	case (pki::sign::SIGN_ID_RSA2048_SHA1):
	case (pki::sign::SIGN_ID_RSA2048_SHA256):
		sign_algo = SIGN_ALGO_RSA2048;
		break;
	case (pki::sign::SIGN_ID_ECDSA240_SHA1):
	case (pki::sign::SIGN_ID_ECDSA240_SHA256):
		sign_algo = SIGN_ALGO_ECDSA240;
		break;
	};

	return sign_algo;
}

pki::sign::HashAlgo pki::sign::getHashAlgo(pki::sign::SignatureId sign_id)
{
	HashAlgo hash_algo = HASH_ALGO_SHA1;

	switch (sign_id)
	{
	case (pki::sign::SIGN_ID_RSA4096_SHA1):
	case (pki::sign::SIGN_ID_RSA2048_SHA1):
	case (pki::sign::SIGN_ID_ECDSA240_SHA1):
		hash_algo = HASH_ALGO_SHA1;
		break;
	case (pki::sign::SIGN_ID_RSA4096_SHA256):
	case (pki::sign::SIGN_ID_RSA2048_SHA256):
	case (pki::sign::SIGN_ID_ECDSA240_SHA256):
		hash_algo = HASH_ALGO_SHA256;
		break;
	};

	return hash_algo;
}