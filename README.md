# UCSC chain/net pipeline

This repository contains chain/net tools extracted
from [kent-core-479](https://github.com/ucscGenomeBrowser/kent-core), which are used for genomic
sequence alignment analysis. Please note that this is not the UCSC Genome Browser source code
repository.

## Build

```bash
cmake -S . -B build
cmake --build build

```

## Tools Overview

### Chain Tools

- `axtChain`: Convert alignments from axt to chain format
- `chainAntiRepeat`: Remove repeats from chain files
- `chainMergeSort`: Merge and sort chain files
- `chainNet`: Create hierarchical net structure from chain files
- `chainPreNet`: Prepare chain file for net creation
- `chainSplit`: Split chain files by chromosome
- `chainStitchId`: Process chain IDs

### Net Tools

- `netChainSubset`: Extract specific chains from nets
- `netFilter`: Filter net files
- `netSplit`: Split net files
- `netSyntenic`: Extract syntenic regions from nets
- `netToAxt`: Convert net to axt format

### Format Conversion Tools

- `axtSort`: Sort axt files
- `axtToMaf`: Convert axt to maf format
- `faToTwoBit`: Convert FASTA to 2bit format
- `lavToPsl`: Convert lav to psl format

## License

Same license as the original kent source.

## References

- [UCSC Genome Browser](https://genome.ucsc.edu/)
- [Data File Formats](https://genome.ucsc.edu/FAQ/FAQformat.html)
