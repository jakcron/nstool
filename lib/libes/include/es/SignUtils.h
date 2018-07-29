#pragma once
#include <es/sign.h>
#include <crypto/sha.h>

namespace es
{

namespace sign
{
	es::sign::SignatureAlgo getSignatureAlgo(es::sign::SignatureId sign_id);
	es::sign::HashAlgo getHashAlgo(es::sign::SignatureId sign_id);
}
	
}