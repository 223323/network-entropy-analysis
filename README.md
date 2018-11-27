
# tools for analyzing pcap network entropy

__Requirements:__
- libpcap
- octave

Use makefile to build project.


```bash
./process.sh --pcap <pcap_filepath> --end-time 60 [-m merge_cmd] [-a entropy_cmd] [-p postfix]
```
