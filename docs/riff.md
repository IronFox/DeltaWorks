# RIFF container (`Core/io/riff_handler.*`)

`DeltaWorks::Riff` reads and writes chunk trees in the Microsoft RIFF format (Resource Interchange File Format, *Multimedia Programming Interface and Data Specifications 1.0*, 1991). It supports two layouts:

- **Standard RIFF**: the root has a form type and every `LIST` has a list type. Written when the root chunk has a form type (`Chunk::SetFormType`).
- **Legacy**: no form type and no list types. This is what DeltaWorks wrote before form types were supported, and what it still writes when no form type is set. Generic RIFF readers misread these files.

Both layouts are read, detected automatically, and load into the same in-memory tree.

Users in this code base and its dependents:

- Hexaverse save games (`.hsf`), standard RIFF with form type `HSF ` from file version 1051 on. Their contents are documented in Hexaverse's [`docs/save-file-format.md`](https://github.com/IronFox/Hexaverse/blob/master/docs/save-file-format.md).
- CGS geometry (`Core/geometry/cgs.*`), which stores meshes, materials, animators and textures in the legacy layout.

## Two classes

| Class | Works on | Used for |
|---|---|---|
| `Riff::Chunk` (alias `Riff::Container`) | An in-memory tree. `LoadFromFile`/`LoadFromData` read a whole file, `SaveToFile`/`SaveToData` write one | Building and reading complete files. Hexaverse save games use only this class |
| `Riff::File` | A file on disk, edited in place through `RandomAccessFile`. Cursor API: `Reset`, `First`, `Next`, `FindFirst`, `Enter`, `DropBack`, `Get`, `AppendBlock`, `InsertBlock`, `DropBlock`, `ResizeBlock` | Reading and patching large files without loading them; CGS loading |

Both read both layouts. `Riff::File` edits files of either layout in place, but `File::Create` always starts a legacy file.

## Byte layout

Every chunk is

```
char  id[4];      // four ASCII characters, e.g. "VERS"
u32   size;       // data size in bytes, not counting the header or the pad byte
u8    data[size];
u8    pad;        // zero, only if size is odd
```

A chunk whose id is `LIST` holds child chunks. The file is one root chunk with the id `RIFF`:

```
standard RIFF                         legacy
"RIFF" u32 size                       "RIFF" u32 size
"HSF "            <- form type        child chunk
child chunk                           "NAME" u32 n <name bytes>
"NAME" u32 n <name bytes>             "LIST" u32 m
"LIST" u32 m                              child chunk
    "WINF"        <- list type            ...
    child chunk                       ...
    ...
...
```

The sizes of the root and of each `LIST` count their form or list type and their children, including the children's pad bytes.

`TID` is the id type. It packs the four characters little-endian into a `UINT32`, so `TID("LIST").value == 0x5453494C` (`RIFF_LIST`), and ids shorter than four characters end early instead of being space-padded: `TID("ID")` is `"ID\0\0"`. Pass `"ID  "` to get a standard space-padded id.

## Form types and list types

- `Chunk::SetFormType(TID)` on the root selects the standard layout. `TID()` selects the legacy layout.
- `Chunk::SetListType(TID)` on a `LIST` chunk sets its list type. In the standard layout, lists without one are written with `RIFF_DEFAULT_LIST_TYPE` (four spaces). In the legacy layout list types are not written.
- Loading sets the root's form type (or none) and each list's type from the file, so loading and saving a file keeps its layout. The children of a loaded `LIST` never include its list type. `GetFormType`/`GetListType` return the types.
- `Riff::File::HasFormType()` and `SubID()` report the form type of an open file. `File::Enter` skips list types, and `AppendBlock`/`InsertBlock` give new lists in a standard file the default list type.

## Detecting the layout

The reader checks the root once. If its id is `RIFF` and bytes 12–15 consist only of uppercase letters, digits and spaces, bytes 8–11 are taken as the form type and the whole file is read as standard RIFF. Otherwise the whole file is read as legacy. (Undefining `RIFF_SENSITIVE` turns the check off, which makes every file legacy.)

In a standard file, bytes 12–15 are the first chunk's id. In a legacy file they are the first chunk's size, which would have to be at least `0x20202020` bytes (about 540 MB) to pass the check. In Hexaverse save files the first chunk is always `VERS` of size 4, so the check is exact there.

Standard files whose first chunk id contains lowercase letters (for example `WAVE` files, which start with `fmt `) are not recognised and are misread as legacy files.

## Comparison with RIFF

| RIFF rule | DeltaWorks | |
|---|---|---|
| File starts with `RIFF` and a 32-bit little-endian size of everything after the size field | Yes. `Chunk::SaveToFile` always writes `RIFF` (the default id of a new `Chunk`); `File::Create` writes `RIFF` and a size of 0, then updates it on every edit | Conforms |
| Byte order of size fields is little-endian | Host byte order (`fwrite` of the header struct). Every platform DeltaWorks builds for is little-endian | Conforms in practice |
| The root's data starts with a 4-character **form type** | Standard layout: yes. Legacy layout: no, the first child chunk starts at offset 8 | Conforms in the standard layout |
| A `LIST`'s data starts with a 4-character **list type** | Standard layout: yes. Legacy layout: no | Conforms in the standard layout |
| Private chunks may carry any data | By convention, a sibling chunk directly before the `LIST` names it. `Chunk::AppendNamedList` and `FindFirstNamedList`/`FindNextNamedList` use a `NAME` chunk with arbitrary bytes; CGS uses a `DATA` chunk with an 8-byte integer | Conforms. To a RIFF reader these are two ordinary sibling chunks. Nothing in the file marks a `NAME` chunk as a list name, and a `NAME` chunk not followed by a `LIST` is ordinary data |
| `LIST` and `RIFF` both contain children | `Chunk` treats only `LIST` (and the root) as a container; a nested `RIFF` chunk is read as data. `File::Enter` enters both | Mostly conforms |
| Data of odd size is followed by one pad byte not counted in `size` | Yes | Conforms |
| The pad byte is zero | Yes. Pad bytes read from older files (which could hold arbitrary values) are replaced by zero | Conforms |
| Ids are alphanumeric ASCII, padded with trailing spaces | Any four bytes without `\0`. The header comment asks for trailing spaces | Caller's responsibility |
| Maximum size 4 GiB | Same. Sizes above `0xFFFFFFFF` call `FATAL__` | Conforms |
| `INFO`, `JUNK` and other registered chunks | No special handling | Neither conforms nor conflicts |

To read a legacy DeltaWorks file with a generic RIFF walker, start at offset 8 instead of 12 and treat every `LIST`'s data as children with no list type.

## Compatibility

DeltaWorks builds from before form types were supported read legacy files only. On a standard file they skip the form type (they already had the detection above), but then read each list type as a chunk header, and `LoadFromFile` throws `FileFormatFault("RIFF size violation")`.
