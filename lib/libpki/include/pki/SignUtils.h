#pragma once
#include <pki/sign.h>
#include <crypto/sha.h>

namespace pki
{

namespace sign
{
	pki::sign::SignatureAlgo getSignatureAlgo(pki::sign::SignatureId sign_id);
	pki::sign::HashAlgo getHashAlgo(pki::sign::SignatureId sign_id);
}
	
}