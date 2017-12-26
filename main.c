//
//  main.c
//  alsatest
//
//  Created by Albin Stigö on 14/12/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

#include "alsa.h"

const int N = 512;
const int FRAME_SIZE = sizeof(float) * 2;

int create_socket_inet(const char *addr, struct sockaddr_in *si_other) {
    int sd = 0;
    ssize_t err = 0;
    static const int send_buf_periods = 4;
    
    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    si_other->sin_family = AF_INET;
    si_other->sin_port = htons(7373);
    
    if (inet_aton(addr, &si_other->sin_addr) == 0) {
        perror("inet_aton");
        exit(EXIT_FAILURE);
    }
    
    int send_buf_size = N * sizeof(float) * send_buf_periods;
    err = setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(send_buf_size));
    if(err < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    return sd;
}

int main(int argc, const char * argv[]) {
    
    int sd = 0;
    
    if (argc < 3) {
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "udpiq DEVICE ADDRESS\n");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in client;
    sd = create_socket_inet(argv[2], &client);
    
    snd_pcm_t *pcm = sdr_pcm_handle(argv[1], N, SND_PCM_STREAM_CAPTURE);
    
    const snd_pcm_channel_area_t *m_areas;
    snd_pcm_uframes_t m_offset, m_frames;
    
    snd_pcm_sframes_t err = 0;
    snd_pcm_sframes_t frames;

    snd_pcm_start(pcm);
    
    while(1) {
        // snd_pcm_wait will select/poll until audio is available
        if((err = snd_pcm_wait(pcm, -1)) < 0) {
            fprintf(stderr, "sndpcm wait %d\n", (int) err);
            snd_pcm_recover(pcm, (int)err, 0);
            continue;
        }
        
        frames = snd_pcm_avail_update(pcm);
        if(frames < 0) {
            fprintf(stderr, "snd_pcm_avail_update\n");
            break;
        }
        
        // m_frames = frames wanted on call and frames available on exit.
        m_frames = frames;
        
        err = snd_pcm_mmap_begin(pcm, &m_areas, &m_offset, &m_frames);
        if(err < 0) {
            perror("snd_pcm_mmap_begin");
            break;
        }

        //printf("wanted: %lu, got: %lu\n", (unsigned long)frames, (unsigned long) m_frames);
        //printf("%lu %lu\n", (unsigned long)m_areas[0].addr, m_offset);

        float *data = &m_areas[0].addr[m_offset * FRAME_SIZE];
        //printf("%f\n", data[100]);
        
        // Send to spectrum socket
        err = sendto(sd,
                     (void*)data,
                     FRAME_SIZE * m_frames,
                     MSG_DONTWAIT, // don't block
                     (struct sockaddr*) &client,
                     sizeof(struct sockaddr));
        
        if(err < 0) {
            perror("send()\n");
        }
        
        // we are finished
        err = snd_pcm_mmap_commit(pcm, m_offset, m_frames);
        if(err < 0) {
            perror("snd_pcm_mmap_commit");
            break;
        }
    }
    
    printf("ended\n");
    
    snd_pcm_close(pcm);

    return 0;
}
