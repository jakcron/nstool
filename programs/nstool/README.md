# Nintendo Switch Tool (NSTool) ![DeviceTag](https://img.shields.io/badge/Device-SWITCH-e60012.svg)
General purpose reading/extraction tool for Nintendo Switch file formats.

## Supported File Formats
* NPDM (.npdm)
* PartitionFS (and HashedPartitionFS) (includes raw .nsp)
* RomFS
* GameCard Image (.xci)
* Nintendo Content Archive (.nca)
* Content Metadata (.cnmt) 
* Nintendo Software Object (.nso) 
* Nintendo Relocatable Software Object (.nro)
* Nintendo Application Control Property (.nacp)
* ES Ticket (v2 only) (.tik)
* PKI Certificate (.cert)

# Usage
```
Usage: nstool [options... ] <file>

  General Options:
      -d, --dev       Use devkit keyset.
      -k, --keyset    Specify keyset file.
      -t, --type      Specify input file type. [xci, pfs, romfs, nca, npdm, cnmt, nso, nro, nacp, aset, cert, tik]
      -y, --verify    Verify file.

  Output Options:
      --showkeys      Show keys generated.
      --showlayout    Show layout metadata.
      -v, --verbose   Verbose output.

  XCI (GameCard Image)
    nstool [--listfs] [--update <dir> --logo <dir> --normal <dir> --secure <dir>] <.xci file>
      --listfs        Print file system in embedded partitions.
      --update        Extract "update" partition to directory.
      --logo          Extract "logo" partition to directory.
      --normal        Extract "normal" partition to directory.
      --secure        Extract "secure" partition to directory.

  PFS0/HFS0 (PartitionFs), RomFs, NSP (Ninendo Submission Package)
    nstool [--listfs] [--fsdir <dir>] <file>
      --listfs        Print file system.
      --fsdir         Extract file system to directory.

  NCA (Nintendo Content Archive)
    nstool [--listfs] [--bodykey <key> --titlekey <key>] [--part0 <dir> ...] <.nca file>
      --listfs        Print file system in embedded partitions.
      --titlekey      Specify title key extracted from ticket.
      --bodykey       Specify body encryption key.
      --tik           Specify ticket to source title key.
      --cert          Specify certificate chain to verify ticket.
      --part0         Extract "partition 0" to directory.
      --part1         Extract "partition 1" to directory.
      --part2         Extract "partition 2" to directory.
      --part3         Extract "partition 3" to directory.

  NSO (Nintendo Software Object), NRO (Nintendo Relocatable Object)
    nstool [--listapi --listsym] [--insttype <inst. type>] <file>
      --listapi       Print SDK API List.
      --listsym       Print Code Symbols.
      --insttype      Specify instruction type [64bit|32bit] (64bit is assumed).

  ASET (Homebrew Asset Blob)
    nstool [--listfs] [--icon <file> --nacp <file> --fsdir <dir>] <file>
      --listfs        Print filesystem in embedded RomFS partition.
      --icon          Extract icon partition to file.
      --nacp          Extract NACP partition to file.
      --fsdir         Extract RomFS partition to directory.
```

# External Keys
NSTool doesn't embed any keys that are copyright protected. However keys can be imported via various keyset files. 

See [SWITCH_KEYS.md](/SWITCH_KEYS.md) for more info.
