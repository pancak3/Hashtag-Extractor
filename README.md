# HashtagExtractor

An Open MPI & OpenMP implementation for extracting and ranking hashtags and languages from a big tweets file.

## Dependency
- mpiCC
- make
- OpenMP

## Usage

Compile and run,
```shell
  make && mpirun -np 4 --bind-to none ./tp $YOUR_TWEETS_FILE_IN_JSON lang.csv
```

_NOTE: In `$YOUR_TWEETS_FILE_IN_JSON` each line should be a tweet following [Twitter Docs](https://developer.twitter.com/en/docs/tweets/data-dictionary/overview/intro-to-tweet-json). The first and the line should not be a tweet line._

## Files
```
.
├── CMakeLists.txt
│       *Directives for cmake
├── combine.cpp
│       *Combine results from multiple processes together
├── combine.hpp
├── include
│   └── rapidjson
│       ├── ... ...
│       └── rapidjson files
├── job.sh
│       *Invoke job.slurm to submmit multiple jobs
├── job.slurm
│       *Slurm script to submmit job to Spartan
├── lang.csv
│       *Pairs of languages and their short name
├── line.cpp
│       *Codes for extracting hashtags and languages from a file
├── line.hpp
├── main.cpp
│       *Main entry to divide a job into sections and assign them to Open MPI processes
├── Makefile
│       *Directives for make
├── results
│   ├── *output files
├── threading.cpp
│       *Codes for each process to divide their job into chunks and process them with OpenMP threads
└── threading.hpp
```
## Third Party Library

- [RapidJson](https://github.com/Tencent/rapidjson)
    Used to parse a JSON string into a document (DOM).