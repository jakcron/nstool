# Nintendo Switch Tool (NSTool) ![DeviceTag](https://img.shields.io/badge/Device-SWITCH-e60012.svg)
General purpose reading/extraction tool for Nintendo Switch file formats.

## Supported File Formats
* PartitionFs (`PFS0`) (.pfs0)
* Sha256PartitionFs (`HFS0`) (.hfs0)
* RomFs (.romfs)
* Nintendo Submission Package (.nsp)
* Nintendo Content Archive (.nca)
* NX GameCard Image (.xci)
* Meta (`META`) (.npdm)
* Nintendo Application Control Property (.nacp)
* Content Metadata (.cnmt) 
* ES Certificate (.cert)
* ES Ticket (v2 only) (.tik)
* Nintendo Shared Object (`NSO0`) (.nso) 
* Nintendo Relocatable Object (`NRO0`) (.nro)
* Initial Program Bundle (`INI1`) (.ini)
* Initial Program (`KIP1`) (.kip)


# Usage
## General usage
The default mode of NSTool is to show general information about a file.

To display general information the usage is as follows:
```
nstool some_file.bin
```

However not all information is shown in this mode; file-layout, key data and properties set to default values are omitted.

## Alternative output modes
To output file-layout information, use the `--showlayout` option:
```
nstool --showlayout some_file.bin
```

To output key data generation and selection, use the `--showkeys` option:
```
nstool --showkeys some_file.bin
```

To output all information, enable the verbose output mode with the `-v` or `--verbose` option:
```
nstool -v some_file.bin
```

## Specify File Type
NSTool will in most cases correctly identify the file type. However you can override this and manually specify the file type with the `-t` or `--intype` option:
```
nstool -t cnmt some_file.bin
```
In that example `cnmt` was selected, NSTool would process the file as `Content Metadata`. See below for a list of supported file type codes:
| Code        | Description |
| ----------- | --------------- |
| gc, xci     | NX GameCard Image |
| nsp         | Nintendo Submission Package |
| pfs         | PartitionFs |
| hfs         | Sha256PartitionFs |
| romfs       | RomFs |
| nca         | Nintendo Content Archive |
| meta, npdm  | Meta (.npdm) |
| cnmt        | Content Metadata |
| nso         | Nintendo Shared Object |
| nro         | Nintendo Relocatable Object |
| ini         | Initial Program Bundle |
| kip         | Initial Program |
| nacp        | Nintendo Application Control Property |
| cert        | ES Certificate |
| tik         | ES Ticket |
| aset, asset | Homebrew NRO Asset Binary |

## Validate Input File
Some file types have signatures/hashes/fields that can be validated by NSTool, but this mode isn't enabled by default.

To validate files with NSTool, enable the verify mode with the `-y` or `--verify` option:
```
nstool -y some_file.bin
```

See the below table for file types that support optional validation:
| File Type | Validation | Comments |
| --------- | ---------- | -------- |
| ES Certificate | Signature | If certificate is part of a certificate chain it will validate it as part of that chain. `Root` signed certificates are verified with user supplied `Root` public key. |
| ES Ticket | Signature | If the user specifies a certificate chain with `--cert` option, the ticket will be verified against that certificate chain. |
| NX GameCard Image | XCI Header Signature, HFS0 Hashes | XCI header signature is verified with user supplied `XCI Header` public key. |
| META | AccessControlInfo fields, AccessControlInfoDesc signature | AccessControlInfo fields are validated against the AccessControlInfoDesc. AccessControlInfoDesc signature is verfied with the appropriate user supplied `ACID` public key. |
| NCA | Header Signature[0], Header Signature[1] | Header Signature[0] is verified with the appropriate user supplied `NCA Header` public key. Header Signature[1] is verified only in Program titles, by retrieving the with public key from the AccessControlInfoDesc stored in the `code` partition. |

* As of Nintendo Switch Firmware 9.0.0, Nintendo retroactively added key generations for some public keys, including `NCA Header` and `ACID` public keys, so the various generations for these public keys will have to be supplied by the user.
* As of NSTool v1.6.0 the public key(s) for `Root Certificate`, `XCI Header`, `ACID` and `NCA Header` are built-in, and will be used if the user does not supply the public key in a key file.

## DevKit Mode
Files generated for `Production` use different (for the most part) encryption/signing keys than files generated for `Development`. NSTool will select `Production` encryption/signing keys by default.
When handling files intended for developer consoles (e.g. systemupdaters, devtools, test builds, etc), you should enable developer mode with the `-d`, `--dev` option:
```
nstool -d some_file.bin
```

## Extract Files
Some file types have some internal file system that can be displayed or extracted

## Encrypted Files
Some Nintendo Switch files are partially or completely encrypted. These require the user to supply the encryption keys to NSTool so that it can process them. 

See [SWITCH_KEYS.md](/SWITCH_KEYS.md) for more info.

# External Keys
NSTool doesn't embed any keys that are copyright protected. However keys can be imported via various keyset files. 

See [SWITCH_KEYS.md](/SWITCH_KEYS.md) for more info.

# Building
See [BUILDING.md](/BUILDING.md).