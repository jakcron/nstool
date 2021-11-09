# Nintendo Switch Tool (NSTool) ![DeviceTag](https://img.shields.io/badge/Device-SWITCH-e60012.svg)
General purpose reading/extraction tool for Nintendo Switch file formats.

## Supported File Formats
* Meta (.npdm)
* PartitionFS (and Sha256PartitionFS) (includes raw .nsp)
* RomFS
* NX GameCard Image (.xci)
* Nintendo Content Archive (.nca)
* Content Metadata (.cnmt) 
* Nintendo Shared Object (`NSO0`) (.nso) 
* Nintendo Relocatable Object (`NRO0`) (.nro)
* Initial Program Bundle (`INI1`) (.ini)
* Initial Program (`KIP1`) (.kip)
* Nintendo Application Control Property (.nacp)
* ES Certificate (.cert)
* ES Ticket (v2 only) (.tik)

# Usage
```
Usage: nstool [options... ] <file>

  General Options:
      -d, --dev       Use devkit keyset.
      -k, --keyset    Specify keyset file.
      -t, --type      Specify input file type. [xci, pfs, romfs, nca, meta, cnmt, nso, nro, ini, kip, nacp, aset, cert, tik]
      -y, --verify    Verify file.

  Output Options:
      --showkeys      Show keys generated.
      --showlayout    Show layout metadata.
      -v, --verbose   Verbose output.

  PFS0/HFS0 (PartitionFs), RomFs, NSP (Nintendo Submission Package)
    nstool [--fstree] [-x [<virtual path>] <out path>] <file>
      --fstree        Print filesystem tree.
      -x, --extract   Extract a file or directory to local filesystem.

  XCI (GameCard Image)
    nstool [--fstree] [-x [<virtual path>] <out path>] <.xci file>
      --fstree        Print filesystem tree.
      -x, --extract   Extract a file or directory to local filesystem.
      --update        Extract "update" partition to directory. (Alias for "-x /update <out path>")
      --logo          Extract "logo" partition to directory. (Alias for "-x /logo <out path>")
      --normal        Extract "normal" partition to directory. (Alias for "-x /normal <out path>")
      --secure        Extract "secure" partition to directory. (Alias for "-x /secure <out path>")

  NCA (Nintendo Content Archive)
    nstool [--fstree] [-x [<virtual path>] <out path>] [--bodykey <key> --titlekey <key> -tik <tik path>] <.nca file>
      --fstree        Print filesystem tree.
      -x, --extract   Extract a file or directory to local filesystem.
      --titlekey      Specify title key extracted from ticket.
      --bodykey       Specify body encryption key.
      --tik           Specify ticket to source title key.
      --cert          Specify certificate chain to verify ticket.
      --part0         Extract partition "0" to directory. (Alias for "-x /0 <out path>")
      --part1         Extract partition "1" to directory. (Alias for "-x /1 <out path>")
      --part2         Extract partition "2" to directory. (Alias for "-x /2 <out path>")
      --part3         Extract partition "3" to directory. (Alias for "-x /3 <out path>")

  NSO (Nintendo Shared Object), NRO (Nintendo Relocatable Object)
    nstool [--listapi --listsym] [--insttype <inst. type>] <file>
      --listapi       Print SDK API List.
      --listsym       Print Code Symbols.
      --insttype      Specify instruction type [64bit|32bit] (64bit is assumed).

  INI (Initial Program Bundle)
    nstool [--kipdir <dir>] <file>
      --kipdir        Extract embedded Inital Programs to directory.

  ASET (Homebrew Asset Blob)
    nstool [--fstree] [-x [<virtual path>] <out path>] [--icon <file> --nacp <file>] <file>
      --fstree        Print RomFS filesystem tree.
      -x, --extract   Extract a file or directory from RomFS to local filesystem.
      --icon          Extract icon partition to file.
      --nacp          Extract NACP partition to file.
```

# External Keys
NSTool doesn't embed any keys that are copyright protected. However keys can be imported via various keyset files. 

See [SWITCH_KEYS.md](/SWITCH_KEYS.md) for more info.

# Building
See [BUILDING.md](/BUILDING.md).