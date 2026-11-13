#ifndef MP3_H
#define MP3_H

void mp3_Start(char *fname, long long *samples, int *rate, int *channels);
void mp3_Stop(void);
int mp3_Update(char *buf, int bytes);

#endif /* MP3_H */
