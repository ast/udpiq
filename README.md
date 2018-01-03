# udpiq

This is a small program for streaming raw IQ samples over UDP. It's part of a larger project to build a
Raspberry Pi based SDR transceiver.

This technique works great over LAN but probably not so great over the internet because of packet loss.

Usable with GNU Radio UDP source (**port 7373 payload size 4096**).

## ideas for improvement aka TODO

* Add assertion that we got the frames we asked for.
* Using `connect` on the UDP socket will make it go faster.
* Reducing UDP packet size to 1472 bytes (because Ethernet 1500 bytes MTU) will probably avoid fragmentation, experiment with `ping`.
* ALSA period size doesn't have to be a power of two. Adapt period size to fit into 1472 bytes.
* Usable with other devices with minimal modification.
* Better to stream ints? For FUNcube Dongle Pro+ it's best to stream shorts.
* Experiment with this over the internet.
* Tag packets to collect statistics. Does GNU Radio have an RTP source?

## building and using

```
# You need to put this in your ~/.asoundrc
# I'm taking advantage of ALSA's built-in
# int to float conversion.
# Otherwise ALSA won't know input is S32_LE.

pcm.sdr {
    type plug
    slave {
        pcm "hw:vfzfpga,0"
        format S32_LE
    }
}
```

```bash
# Then use like this:
$ ./udpiq sdr IP_TO_STREAM_TO
```
