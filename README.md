# NNTools

Tools & Libraries for Nintendo devices.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](/LICENSE)

# Tools
* [__nstool__](/programs/nstool/README.md) - General purpose reading/extraction tool for Nintendo Switch file formats.

# Libraries
* __libfnd__ - Foundation library.
* __libcrypto__ - Cryptographic functions (AES,SHA,RSA). Wrapper for [mbedTLS](https://github.com/ARMmbed/mbedtls)
* __libcompress__ - Compression algorithms (LZ4). Wrapper for [lz4](https://github.com/lz4/lz4)
* __libpki__ - Processes Nintendo's proprietary PKI.
* __libes__ - Processes Nintendo's eShop file types.
* __libhac__  - Processes Nintendo Switch file types.
* __libhac-hb__ - Processes Nintendo Switch file types (homebrew extensions).

# Building
On MacOS/Linux/WSL run `make`.

For Windows, Visual Studio 2017 is supported.