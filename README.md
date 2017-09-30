# LZ78
Implementation of one of the most important lossless algorithm for file/stream compression. LZ78 (Lempel-Ziv 1978) is a dictionary based compressor that implements a tree based dictionary built at runtime ([Wikipedia](https://en.wikipedia.org/wiki/LZ77_and_LZ78#LZ78)).

### Features:
- Tree-based dictionary implemented by means of an open hash table;
- Both little-endian processor as well as big-endian processor are supported;
- "User-friendly" command line interface with help menu;
- Customizable dictionary length (max 4294967296 symbols);
