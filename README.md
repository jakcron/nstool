# NXTools
Tools for NX (Nintendo Switch).

== Tools ==
* ncatool - read/extract *.nca
* npdmtool - read *.npdm files
* pfstool - read/extract PartitionFS blobs (including *.nsp).

== Issues ==
* [ncatool] AES-XTS not implemented properly, breaks decryption for nca header sectors 2,3,4,5
* [ncatool] nca section crypto not reverse engineered, so they are not decrypted when extracted.
