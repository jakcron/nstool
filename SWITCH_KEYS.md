
# About
Keyset files are ASCII text files containing one key per line, in the form "__key_name = HEXADECIMALKEY__". Case and whitespace do not matter, ';' character can be used to insert comments.

# Loading keyset files automatically
If a keyset file is located in ___$HOME/.switch/___ it will be loaded automatically.

# General Keys (prod.keys and dev.keys)
Some switch files formats feature encryption and or cryptographic signatures. In order to process these file formats, some keys are required. These keys can be supplied via a keyfile: ___prod.keys___ (or ___dev.keys___ for devkit variants).

This keyset file can be provided via the command line, use the `-k` or `--keyset` option:
```
nstool -k prod.keys some_file.bin
```

## Format
The following keys are recognised (## represents the key revision, a hexadecimal number between 00 and FF):

```
; Key Sources
master_key_##                         : Master key, used to derive other keys. (0x10 bytes)
aes_kek_generation_source             : Used to derive other aes-keks. (0x10 bytes)
aes_key_generation_source             : Used to derive other aes-keys. (0x10 bytes)
package2_key_source                   : Used with master_key_## to derive package2_key_##. (0x10 bytes)
ticket_commonkey_source               : Used with master_key_## to derive ticket_commonkey_##. (0x10 bytes)
nca_header_kek_source                 : Used with master_key_00, aes_kek_generation_source and aes_key_generation_source to generate nca_header_kek.  (0x10 bytes)
nca_header_key_source                 : Used with nca_header_kek to generate nca_header_key.  (0x20 bytes)
nca_key_area_key_application_source   : Used with master_key_##, aes_kek_generation_source and aes_key_generation_source to generate nca_key_area_key_application_##. (0x10 bytes)
nca_key_area_key_ocean_source         : Used with master_key_##, aes_kek_generation_source and aes_key_generation_source to generate nca_key_area_key_ocean_##. (0x10 bytes)
nca_key_area_key_system_source        : Used with master_key_##, aes_kek_generation_source and aes_key_generation_source to generate nca_key_area_key_system_##. (0x10 bytes)

; Package1 keys
package1_key_##                       : AES128 Key (0x10 bytes)

; Package2 Keys 
package2_key_##                       : AES128 Key (0x10 bytes)
package2_sign_key_modulus             : RSA2048 Modulus (0x100 bytes)
package2_sign_key_private             : RSA2048 Private Exponent (0x100 bytes)

; Ticket Keys
ticket_commonkey_##                   : AES128 Key (0x10 bytes)

; PKI Root Signing Key
pki_root_sign_key_modulus             : RSA4096 Modulus (0x200 bytes)
pki_root_sign_key_private             : RSA4096 Private Exponent (0x200 bytes)

; NCA Keys
nca_header_key                        : AES128-XTS Key (0x20 bytes)
nca_header_sign_key_##_modulus        : RSA2048 Modulus (0x100 bytes)
nca_header_sign_key_##_private        : RSA2048 Private Exponent (0x100 bytes)
nca_key_area_key_application_##       : AES128 Key (0x10 bytes)
nca_key_area_key_ocean_##             : AES128 Key (0x10 bytes)
nca_key_area_key_system_##            : AES128 Key (0x10 bytes)
nca_key_area_key_hw_application_##    : AES128 Key (0x10 bytes)
nca_key_area_key_hw_ocean_##          : AES128 Key (0x10 bytes)
nca_key_area_key_hw_system_##         : AES128 Key (0x10 bytes)

; NRR Keys
nrr_certificate_sign_key_##_modulus   : RSA2048 Modulus (0x100 bytes)
nrr_certificate_sign_key_##_private   : RSA2048 Private Exponent (0x100 bytes)

; XCI Keys
xci_header_key_##                     : AES128 Key (0x10 bytes)
xci_header_sign_key_modulus           : RSA2048 Modulus (0x100 bytes)
xci_header_sign_key_private           : RSA2048 Private Exponent (0x100 bytes)
xci_initial_data_kek_##               : AES128 Key (0x10 bytes)
xci_cert_sign_key_modulus             : RSA2048 Modulus (0x100 bytes)
xci_cert_sign_key_private             : RSA2048 Private Exponent (0x100 bytes)

; ACID Keys
acid_sign_key_##_modulus              : RSA2048 Modulus (0x100 bytes)
acid_sign_key_##_private              : RSA2048 Private Exponent (0x100 bytes)
```

## Legacy Keynames
Since firmware `9.0.0+` support for signature key generations was retroactively added for RSA-PSS signatures in NRR, ACID and NCA. The old names for these keys are still valid:
```
nca_header_sign_key_modulus      : alias for nca_header_sign_key_00_modulus
nca_header_sign_key_private      : alias for nca_header_sign_key_00_private
acid_sign_key_modulus            : alias for acid_sign_key_00_modulus
acid_sign_key_private            : alias for acid_sign_key_00_private
```

## Compatibility with hactool keyset files
`prod.keys` (and `dev.keys`) keyset files share the same keyset file format as [hactool](https://github.com/SciresM/hactool/blob/master/KEYS.md), but names of keys may differ. For compatibility, hactool names for equivalent keys are accepted.
```
titlekek_source                  : hactool alias for ticket_commonkey_source
header_key_source                : hactool alias for nca_header_key_source
header_kek_source                : hactool alias for nca_header_kek_source
key_area_key_application_source  : hactool alias for nca_key_area_key_application_source
key_area_key_ocean_source        : hactool alias for nca_key_area_key_ocean_source
key_area_key_system_source       : hactool alias for nca_key_area_key_system_source 
titlekek_##                      : hactool alias for ticket_commonkey_##
header_key                       : hactool alias for nca_header_key
key_area_key_application_##      : hactool alias for nca_key_area_key_application_##
key_area_key_ocean_##            : hactool alias for nca_key_area_key_ocean_##
key_area_key_system_##           : hactool alias for nca_key_area_key_system_##
```

## Encrypted File Types
See below for advice on what keys are required to decrypt certain file types.

### NX GameCard Image
The `GameCard ExtendedHeader` is encrypted with one of 8 keys, specified by the `KekIndex` in the `GameCard Header`.
It isn't required to extract game data, it just contains metadata.

Only two keys are currently defined:
| KeyIndex    | Name            | Description |
| ----------- | --------------- | ----------- |
| 00          | Production      | Usually selected for prod images. Some dev images use this key index. |
| 01          | Development     | Usually selected for dev images. This was changed from key index 00 at some point. |

Define the header key(s) in `prod.keys`/`dev.keys` (Prod and dev share the same keydata):
```
xci_header_key_00 = <32 char AES128 key here>
xci_header_key_01 = <32 char AES128 key here>
```

### Nintendo Content Archive
Nintendo Content Archive (NCA) files have both an encrypted header and content. The encrypted header determines the layout/format/encryption method of the content, which contains the game data.

Define the header key in `prod.keys`/`dev.keys`.
```
nca_header_key = <64 char AES128-XTS key-data here>
```
Or allow NSTool to derive it from key sources:
```
master_key_00             = <32 char AES128 key-data here>
aes_kek_generation_source = <32 char AES128 key-data here>
aes_key_generation_source = <32 char AES128 key-data here>
nca_header_kek_source     = <32 char AES128 key-data here>
nca_header_key_source     = <64 char AES128 key-data here>
```

In order to read the NCA content, the content key must be determined. Unlike the header key which is fixed, each NCA will have a unique content key.

Content keys are either:
1) "Internal", where they are encrypted the NCA Header KeyArea
2) "External", where they are encrypted in an external Ticket file (.tik) (external content keys are sometimes called title keys)

#### Internal Content Key
Decrypting the content key from the NCA Header Key Area requires the appropriate `nca_key_area_key` to be defined in `prod.keys`/`dev.keys`.
However for security reasons Nintendo revises this key periodically, and within each key revision there are 3 separate keys for different categories of applications.

It's best to define as many of these as possible, to reduce the number of times you need to edit the keyfiles.

So for a given key revision these key area keys can be defined explicitly (`##` represents the key revision in hexadecimal):
```
nca_key_area_key_application_## = <32 char AES128 key-data here>
nca_key_area_key_ocean_##       = <32 char AES128 key-data here>
nca_key_area_key_system_##      = <32 char AES128 key-data here>
```
Or allow NSTool to derive them from key sources: (`##` represents the key revision in hexadecimal):
```
master_key_##                       = <32 char AES128 key-data here>
aes_kek_generation_source           = <32 char AES128 key-data here>
aes_key_generation_source           = <32 char AES128 key-data here>
nca_key_area_key_application_source = <32 char AES128 key-data here>
nca_key_area_key_ocean_source       = <32 char AES128 key-data here>
nca_key_area_key_system_source      = <32 char AES128 key-data here>
```

#### External Content Key
For NCAs that use an external content key, the user must supplied the key to NSTool.

Most NCAs that use an external content key will be bundled with a ticket file (*.tik) that contains the content key in an encrypted form.

The ticket can be supplied by the user using the `--tik` option:
```
nstool --tik <32 char rightsid>.tik <32 char contentid>.nca
```
This however requires the the appropriate commonkey to be defined in `prod.keys`/`dev.keys` to decrypt the content key in the ticket. However for security reasons Nintendo revises this key periodically. 

It's best to define as many of these as possible, to reduce the number of times you need to edit the keyfiles.

So for a given key revision the common key can be defined explicitly (`##` represents the key revision in hexadecimal):
```
ticket_commonkey_## = <32 char AES128 key-data here>
```
Or allow NSTool to derive them from key sources: (`##` represents the key revision in hexadecimal):
```
master_key_##           = <32 char AES128 key-data here>
ticket_commonkey_source = <32 char AES128 key-data here>
```

##### Supply the external content key directly to NSTool
Alternatively you can supply the raw encrypted content key (also called a title key) directly with the `--titlekey` option:
```
nstool --titlekey <32 char AES128 key-data here> <32 char contentid>.nca
```

It is also possible to supply the decrypted content key directly with the `--contentkey` option:
```
nstool --contentkey <32 char AES128 key-data here> <32 char contentid>.nca
```

##### Scene Tickets
Please note that "Scene" tickets have been known to have errors. If you have issues using the `--tik` option, try passing the raw encrypted titlekey directly with the `--titlekey` option. The titlekey can be found by reading the ticket with NSTool:
```
nstool <32 char rightsid>.tik
```

##### Personalised Tickets
If the ticket is personalised (encrypted with console unique RSA key), NSTool will not support it. You will need to use extract the title key with another tool and pass the encrypted title key directly with the `--titlekey` option.

# Title Keys (title.keys)
In order for NSTool to decrypt NCA files that use external content keys, the ticket or key data be provided to NSTool. For convience NSTool supports the hactool `title.keys` format. This file can store a dictionary of title keys, so that specifying a ticket or key data manually is not required, provided it is present in `title.keys`. This file must be present in: ___$HOME/.switch/___ .

## Format
* This file is in the format of (rights_id = title_key) pairs, each on their own line. 
* There is no limit on the number of pairs. 
* The `;` is the comment indicator. When parsing a file, it is treated as a new line character.
* The format is case insensitive


### Example
For example if rights id `010003000e1468000000000000000008` had a title key `8fa820b219781d331cca08968e6e5b52`, the row would look like this:
```
010003000e1468000000000000000008 = 8fa820b219781d331cca08968e6e5b52
```