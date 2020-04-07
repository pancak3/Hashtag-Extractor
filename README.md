# Hashtag Extractor
An OpenMPI & OpenMP solution for extracting and ranking hashtags and languages from tweets.

## Dependency
- mpiCC
- make
- OpenMP

## Third Party Dependencies (included) 
- [RapidJson](https://github.com/Tencent/rapidjson)
    Used to parse a JSON string into a document (DOM).

## Usage
Compile and run,
```shell
  make && mpirun -np 4 --bind-to none ./tp <tweets.json> lang.csv
```

_NOTE: In `<tweets.sh>`, each line should be a tweet following the format specified in [Twitter Docs](https://developer.twitter.com/en/docs/tweets/data-dictionary/overview/intro-to-tweet-json). The first and last lines should not be tweets._

## Files
```
.
├── combine.cpp
│       * Combine results from multiple processes together
├── combine.hpp
├── include
│   └── rapidjson
│       └── rapidjson files
├── job.sh
│       * Invokes job.slurm to submit multiple jobs
├── job.slurm
│       * Slurm script to submit job to Spartan HPC
├── lang.csv
│       * Mappings between languages and language codes
├── line.cpp
│       * Extracts hashtags and languages from tweets (in JSON form)
├── line.hpp
├── main.cpp
│       * Entrypoint of program, divides the input file into sections and assign them to MPI processes
├── Makefile
│       * Directives for make
├── results
│   ├── * Output files (results) from Spartan
├── threading.cpp
│       * Each process further subdivides their assigned sections into chunks and process them with OpenMP threads
└── threading.hpp
```

