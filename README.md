# NXTools

Tools & Libraries for NX (Nintendo Switch).

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](/LICENSE)

# Tools

* __nstool__ - read *.npdm, read/extract PartitionFS (PFS0|HFS0) blobs (including *.nsp), read *.xci
* __ncatool__ - read/extract *.nca
* __tiktool__ - read ticket fields.

# Libraries

* __libfnd__	- Foundation library.
* __libcrypto__	- Cryptographic functions (AES,SHA,RSA). Wrapper for [mbedTLS](https://github.com/ARMmbed/mbedtls)
* __libes__		- Handling of (NS relevant) eShop file type processing. (eTickets, etc)
* __libnx__		- Handling of NS file types

# External Keys

Programs/libraries don't embed any keys that are copyright protected. However keys can be imported via a keyset file. 

For programs that support it, the keyset file can be provided via the command line (see program usage for details). Alternatively a keyset file located in ___$HOME/.switch/prod.keys___ (or ___$HOME/.switch/dev.keys___ for dev) will be imported automatically if one is not provided at the command line.

Keyset files are ASCII text files containing one key per line, in the form "__key_name = HEXADECIMALKEY__". Case and whitespace do not matter, ';' character can be used to insert comments.

See [KEYS.md](/KEYS.md) for more details.