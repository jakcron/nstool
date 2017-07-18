# NXTools
Tools for NX (Nintendo Switch).

I started writing tools for the Nintendo Switch, but development has stopped. This is provided "as-is".

== Tools ==
* ncatool - read/extract *.nca
* npdmtool - read *.npdm files
* pfstool - read/extract PartitionFS blobs.

== Issues ==
* [ncatool] AES-XTS not implemented properly, breaks decryption for nca header sectors 2,3,4,5
* [ncatool] nca section crypto not reverse engineered, so they are not decrypted when extracted.
