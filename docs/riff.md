# RIFF container (`Core/io/riff_handler.*`)

`DeltaWorks::Riff` reads and writes a chunk tree modelled on the Microsoft RIFF format (Resource Interchange File Format, *Multimedia Programming Interface and Data Specifications 1.0*, 1991). It keeps RIFF's chunk framing but leaves out the form type and list types, and names lists differently. Files it writes are therefore **RIFF-like, not RIFF**: generic RIFF readers misparse them, and it reads standard RIFF files (WAVE, AVI …) only in simple cases.

Users in this code base and its dependents:

- Hexaverse save games (`.hsf`). Their contents are documented in Hexaverse's [`docs/save-file-format.md`](https://github.com/IronFox/Hexaverse/blob/master/docs/save-file-format.md).
- CGS geometry (`Core/geometry/cgs.*`), which stores meshes, materials, animators and textures in it.

## Two classes

| Class | Works on | Used for |
|---|---|---|
| `Riff::Chunk` (alias `Riff::Container`) | An in-memory tree. `LoadFromFile`/`LoadFromData` read a whole file, `SaveToFile`/`SaveToData` write one | Building and reading complete files. Hexaverse save games use only this class |
| `Riff::File` | A file on disk, edited in place through `RandomAccessFile`. Cursor API: `Reset`, `First`, `Next`, `FindFirst`, `Enter`, `DropBack`, `Get`, `AppendBlock`, `InsertBlock`, `DropBlock`, `ResizeBlock` | Reading and patching large files without loading them; CGS loading |

Both produce and accept the same byte layout.

## Byte layout

Every chunk is

```
char  id[4];      // four ASCII characters, e.g. "VERS"
u32   size;       // data size in bytes, not counting the header or the pad byte
u8    data[size];
u8    pad;        // only if size is odd
```

A chunk whose id is `LIST` holds child chunks as its data, back to back. The file is one root chunk with the id `RIFF` whose data are the top-level children:

```
"RIFF" u32 size
    child chunk
    "NAME" u32 n  <name bytes>     <- names the following list
    "LIST" u32 m
        child chunk
        ...
    ...
```

`TID` is the id type. It packs the four characters little-endian into a `UINT32`, so `TID("LIST").value == 0x5453494C` (`RIFF_LIST`), and ids shorter than four characters end early instead of being space-padded: `TID("ID")` is `"ID\0\0"`. Pass `"ID  "` to get a standard space-padded id.

## Comparison with RIFF

| RIFF rule | DeltaWorks | |
|---|---|---|
| File starts with `RIFF` and a 32-bit little-endian size of everything after the size field | Yes. `Chunk::SaveToFile` always writes `RIFF` (the default id of a new `Chunk`); `File::Create` writes `RIFF` and a size of 0, then updates it on every edit | Conforms |
| Byte order of size fields is little-endian | Host byte order (`fwrite` of the header struct). Every platform DeltaWorks builds for is little-endian | Conforms in practice |
| The root's data starts with a 4-character **form type** | **Not written.** The first child chunk starts at offset 8. On reading, both classes guess: if the root is `RIFF` and bytes 12–15 consist only of uppercase letters, digits and spaces, bytes 8–11 are taken as a form type and skipped (`RIFF_SENSITIVE`, on by default). `File::SubID()` returns it; `Chunk` discards it. `Chunk::LoadFromData` does not make this check | Diverges. Standard readers take the first child's id as the form type. The guess fails for standard files whose first chunk id has lowercase letters (`WAVE` followed by `fmt `) |
| A `LIST`'s data starts with a 4-character **list type** | **Not written or expected.** The list's data are its children | Diverges. A standard `LIST` (with a list type) is misread: its list type is parsed as the start of a chunk header |
| Lists are identified by their list type | By convention, a sibling chunk directly before the `LIST` names it. `Chunk::AppendNamedList` and `FindFirstNamedList`/`FindNextNamedList` use a `NAME` chunk with arbitrary bytes; CGS uses a `DATA` chunk with an 8-byte integer | Diverges. The pairing is only a convention: nothing in the file marks a `NAME` chunk as a list name, and a `NAME` chunk not followed by a `LIST` is ordinary data |
| `LIST` and `RIFF` both contain children | `Chunk` treats only `LIST` (and the root) as a container; a nested `RIFF` chunk is read as data. `File::Enter` enters both | Mostly conforms |
| Data of odd size is followed by one pad byte not counted in `size` | Yes. A list's size includes its children's pad bytes | Conforms |
| The pad byte is zero | **Not initialised.** The data buffer is allocated with room for the pad byte but only `size` bytes are copied, so the pad byte holds whatever was in memory (`0xCD` in MSVC debug builds). `File::ResizeBlock` zero-fills only when growing | Diverges. Readers ignore the value, but it can leak a byte of heap memory into the file |
| Ids are alphanumeric ASCII, padded with trailing spaces | Any four bytes without `\0`. The header comment asks for trailing spaces | Caller's responsibility |
| Maximum size 4 GiB | Same. Sizes above `0xFFFFFFFF` call `FATAL__` | Conforms |
| `INFO`, `JUNK` and other registered chunks | No special handling | Neither conforms nor conflicts |

To read a DeltaWorks file with a generic RIFF walker, start at offset 8 instead of 12 and treat every `LIST`'s data as children with no list type.

## Known issues in the implementation

These do not affect files written and read through `Chunk::SaveToFile`/`LoadFromFile`, which is the path Hexaverse uses:

- `Chunk::FromData` (used by `LoadFromData`) never decreases its remaining-size counter while reading a list's children, so loading a list from memory does not stop at the end of the list.
- `Chunk::FindNextNamedListMatch` has a stray `;` after its `while`, so it runs the loop to the end and then dereferences the last (null) result.
- `File::AppendBlock` and `File::InsertBlock` copy the size as a `size_t`, which on 64-bit builds writes 8 bytes into the 4-byte size field and past the end of the buffer when the data is shorter than 4 bytes. The value stored in the file is still correct.
