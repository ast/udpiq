# udpiq

This is a small program to stream raw IQ samples over UDP. This technique works great over LAN but
probably wont be that great over the internet because of packet loss.

Usable with Gnuradio UDP source (**port 7373 payload size 4096**).

```
# You need to put this in your ~/.asoundrc
# I'm taking advantage of ALSAs builtin
# int -> float conversion.

pcm.sdr {
    type plug
    slave {
        pcm "hw:vfzfpga,0"
        rate 39000
        format S32_LE
    }
}
```

```bash
# Then use like this:
$ ./udpiq sdr IP_TO_STREAM_TO
```
