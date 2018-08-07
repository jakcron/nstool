# NNTools

Tools & Libraries for Nintendo devices.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](/LICENSE)

# Tools
* [__nstool__](/programs/nstool/README.md) - General purpose read/extract tool for Nintendo Switch file formats.

# Libraries
* __libfnd__ - Foundation library. Provides file IO, crypto, CLI utils, string conversion, etc.
* __libpki__ - Processes Nintendo's proprietary PKI.
* __libes__ - Processes Nintendo's eShop file formats.
* __libhac__  - Processes Nintendo Switch file formats.
* __libhac-hb__ - Processes Nintendo Switch file formats (homebrew extensions).

# Dependencies
* __libpolarssl__ - Cryptographic functions (AES,SHA,RSA). Clone of [polarssl](https://github.com/ARMmbed/mbedtls) (now mbedTLS).
* __liblz4__ - Compression algorithms (LZ4). Clone of [lz4](https://github.com/lz4/lz4).

(Copies of these are included locally and are statically linked libraries)

# Building
On MacOS/Linux/WSL run `make`.

For Windows, Visual Studio 2017 is supported.