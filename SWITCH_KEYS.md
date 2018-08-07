
# About
Keyset files are ASCII text files containing one key per line, in the form "__key_name = HEXADECIMALKEY__". Case and whitespace do not matter, ';' character can be used to insert comments.

# Loading keyset files automatically
If a keyset file is located in ___$HOME/.switch/___ it will be loaded automatically.

# General Keys (prod.keys and dev.keys)
Some switch files formats feature encryption and or cryptographic signatures. In order to process these file formats, some keys are required. These keys can be supplied via a keyfile: ___prod.keys___ (or ___dev.keys___ for devkit variants).

This keyset file can be provided via the command line (refer to usage for details).



## Format
The following keys are recognised (## represents a hexadecimal number between 00 and 1F):

```
; Key Sources
master_key_##                    : Master key, used to derive other keys. (0x10 bytes)
aes_kek_generation_source        : Used to derive other aes-keks. (0x10 bytes)
aes_key_generation_source        : Used to derive other aes-keys. (0x10 bytes)
package2_key_source              : Used with master_key_## to derive package2_key_##. (0x10 bytes)
ticket_commonkey_source          : Used with master_key_## to derive ticket_commonkey_##. (0x10 bytes)
nca_header_kek_source            : Used with master_key_00, aes_kek_generation_source and aes_key_generation_source to generate nca_header_kek.  (0x10 bytes)
nca_header_key_source            : Used with nca_header_kek to generate nca_header_key.  (0x20 bytes)
nca_body_keak_application_source : Used with master_key_##, aes_kek_generation_source and aes_key_generation_source to generate nca_body_keak_application_##. (0x10 bytes)
nca_body_keak_ocean_source       : Used with master_key_##, aes_kek_generation_source and aes_key_generation_source to generate nca_body_keak_ocean_##. (0x10 bytes)
nca_body_keak_system_source      : Used with master_key_##, aes_kek_generation_source and aes_key_generation_source to generate nca_body_keak_system_##. (0x10 bytes)

; Package1 keys
package1_key_##                  : AES128 Key (0x10 bytes)

; Package2 Keys 
package2_key_##                  : AES128 Key (0x10 bytes)
package2_sign_key_modulus        : RSA2048 Modulus (0x100 bytes)
package2_sign_key_private        : RSA2048 Private Exponent (0x100 bytes)

; Ticket Keys
ticket_commonkey_##              : AES128 Key (0x10 bytes)

; PKI Root Signing Key
pki_root_sign_key_modulus        : RSA4096 Modulus (0x200 bytes)
pki_root_sign_key_private        : RSA4096 Private Exponent (0x200 bytes)

; NCA Keys
nca_header_key                   : AES128-XTS Key (0x20 bytes)
nca_header_sign_key_modulus      : RSA2048 Modulus (0x100 bytes)
nca_header_sign_key_private      : RSA2048 Private Exponent (0x100 bytes)
nca_body_keak_application_##     : AES128 Key (0x10 bytes)
nca_body_keak_ocean_##           : AES128 Key (0x10 bytes)
nca_body_keak_system_##          : AES128 Key (0x10 bytes)

; XCI Keys
xci_header_key                   : AES128 Key (0x10 bytes)
xci_header_sign_key_modulus      : RSA2048 Modulus (0x100 bytes)
xci_header_sign_key_private      : RSA2048 Private Exponent (0x100 bytes)

; ACID Keys
acid_sign_key_modulus            : RSA2048 Modulus (0x100 bytes)
acid_sign_key_private            : RSA2048 Private Exponent (0x100 bytes)
```

## Compatibility with hactool keyset files
`prod.keys` (and `dev.keys`) keyset files share the same keyset file format as [hactool](https://github.com/SciresM/hactool/blob/master/KEYS.md), but names of keys may differ. For compatibility, hactool names for equivalent keys are accepted.
```
titlekek_source                  : hactool alias for ticket_commonkey_source
header_key_source                : hactool alias for nca_header_key_source
header_kek_source                : hactool alias for nca_header_kek_source
key_area_key_application_source  : hactool alias for nca_body_keak_application_source
key_area_key_ocean_source        : hactool alias for nca_body_keak_ocean_source
key_area_key_system_source       : hactool alias for nca_body_keak_system_source 
titlekek_##                      : hactool alias for ticket_commonkey_##
header_key                       : hactool alias for nca_header_key
key_area_key_application_##      : hactool alias for nca_body_keak_application_##
key_area_key_ocean_##            : hactool alias for nca_body_keak_ocean_##
key_area_key_system_##           : hactool alias for nca_body_keak_system_##
```