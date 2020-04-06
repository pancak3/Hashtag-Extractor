# HashtagExtractor

An Open MPI & OpenMP implementation for extracting and ranking hashtags and languages from a big tweets file.

## Dependency
- mpiCC

## Usage

Compile and run,
```shell
  make && mpirun -np 4 --bind-to none ./tp $YOUR_TWEETS_FILE_IN_JSON lang.csv
```
Outputs,

```
mpiCC -fopenmp -O3 -std=c++11 -o tp combine.o threading.o line.o main.cpp
[*] File $YOUR_TWEETS_FILE_IN_JSON (in bytes):  23920179

[*] Language Freq Results
English (en), 4,127
undefined (und), 302
Portuguese (pt), 101
French (fr), 92
Thai (th), 83
Spanish (es), 61
Japanese (ja), 32
Tagalog (tl), 27
Indonesian (in), 22
Korean (ko), 22

[*] Hashtag Freq Results
#climatechange, 39
#auspol, 34
#scottyfrommarketing, 29
#arrestus, 24
#twitterleft, 24
#babe, 21
#resist, 19
#9news, 18
#whistleblower, 15
#1, 15
#australianfires, 15

[*] Time cost (built-in)
        0.51253 seconds
```
_NOTE: In `$YOUR_TWEETS_FILE_IN_JSON` each line should be a tweet following [Twitter Docs](https://developer.twitter.com/en/docs/tweets/data-dictionary/overview/intro-to-tweet-json). The first and the last line should not be a tweet line._

## License
[License](LICENSE.MD)
