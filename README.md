# NXTools

Tools & Libraries for NX (Nintendo Switch).

![License](https://img.shields.io/badge/license-MIT-blue.svg)

# Tools

* nstool - read *.npdm, read/extract PartitionFS (PFS0|HFS0) blobs (including *.nsp), read *.xci
* ncatool - read/extract *.nca
* tiktool - read ticket fields.

# Libraries

* libfnd	- Foundation library.
* libcrypto	- Cryptographic functions (AES,SHA,RSA). Wrapper for [mbedTLS](https://github.com/ARMmbed/mbedtls)
* libes		- Handling of (NS relevant) eShop file type processing. (eTickets, etc)
* libnx		- Handling of NS file types

# External Keys

Programs/libraries don't embed any keys that are copyright protected. However keys can be imported via a keyset file. 

For programs that support it, the keyset file can either be provided via the command line (see program usage for details). Alternatively a keyset file located in _$HOME/.switch/prod.keys_ (or _$HOME/.switch/dev.keys_ for dev) will be imported automatically if one is not provided at the command line.

Keyset files are ASCII text files containing one key per line, in the form "key_name = HEXADECIMALKEY". Case and whitespace do not matter, ';' character can be used to insert comments.

See [KEYS.md](/KEYS.md) for more details.