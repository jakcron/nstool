#pragma once
#include <nn/pki/sign.h>
#include <fnd/sha.h>

namespace nn
{
namespace pki
{

namespace sign
{
	nn::pki::sign::SignatureAlgo getSignatureAlgo(pki::sign::SignatureId sign_id);
	nn::pki::sign::HashAlgo getHashAlgo(pki::sign::SignatureId sign_id);
}
	
}
}