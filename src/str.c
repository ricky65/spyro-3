#include "common.h"
#include "ovl_header.h"

// externs
extern int func_8005D96C(int,void*,unsigned int); // probably equivalent to CdRead from Spyro 1
extern int func_8005E0BC(unsigned char, CdLoc*, char*); // probably equivalent to CdControl from Spyro 1

// sdata
extern int speechLba; // 8006C3F4 - should be 90000
extern int D_8006C674; // 8006C674 - moby speech index to play (entry in speech list)
extern int language; // 8006C76C

// bss
extern CDState cdState; // 8006e470
extern StreamingData streamingData; // 8006e470 - would be 8006e48c if CDState and StreamingData are split

////////////////////////////////////////////////////////////////////////////////////

/**
 * ???() - func_8004F8EC() - MATCHING
 * https://decomp.me/scratch/y8iwN
 */
void func_8004F8EC() {
    int* temp_v0;
    int* temp_v1;

    streamingData.dat_8006e48c = 0;
    streamingData.musicEnabled = 1;
    streamingData.dat_8006e490 = 0;
    streamingData.dat_8006e494 = 0;
    streamingData.dat_8006e498 = 0;
    
    streamingData.musicVolume = 0x5FFF;
    streamingData.speechVolume = 0x7FFF;
    temp_v0 = &streamingData.musicVolume;
    temp_v1 = &streamingData.speechVolume;
    streamingData.dat_8006e4b4 = 0;
    
    streamingData.dat_8006e4b8.unk0 = 0;
    streamingData.musicData.unk0 = 0;
    streamingData.dat_8006e4e0.unk0 = 0;
    streamingData.speechData.unk0 = 0;
    
    streamingData.dat_8006e4b8.volumePtr = temp_v0;
    streamingData.musicData.volumePtr = temp_v0;
    streamingData.dat_8006e4e0.volumePtr = temp_v1;
    streamingData.speechData.volumePtr = temp_v1;
}

/**
 * ???() - func_8004F984() - MATCHING
 * Seems to be a music init function
 * https://decomp.me/scratch/OgH2g
 */
void func_8004F984(int startLba, int endLba, int track) {
    streamingData.musicData.unk0 = startLba;
    streamingData.musicData.startLba = startLba;
    streamingData.musicData.endLba = endLba;
    streamingData.musicData.track = track;
    streamingData.dat_8006e48c = 8;
    streamingData.musicEnabled = 0;
}

/**
 * ???() - func_8004F9C0() - MATCHING
 * Seems to be a dialogue init function
 * https://decomp.me/scratch/FCfLr
 */
void func_8004F9C0(int startLba, int endLba, int track) {
    if (*streamingData.speechData.volumePtr > 0) {
        streamingData.speechData.unk0 = startLba;
        streamingData.speechData.startLba = startLba;
        streamingData.speechData.endLba = endLba;
        streamingData.speechData.track = track;
        if (streamingData.dat_8006e48c != 5) {
            streamingData.dat_8006e48c = 8;
        }
        streamingData.musicEnabled = 0;
    }
}

/**
 * ???() - func_8004FA24()
 * TODO
 */
INCLUDE_ASM("asm/nonmatchings/str", func_8004FA24);

/**
 * ???() - func_800503F8() - MATCHING
 * Looks equivalent to CDLoadTime in Spyro 1
 * https://decomp.me/scratch/jQNst
 */
int func_800503F8(void) {
    unsigned char modeFlags;
    int CdlComplete;

    if (streamingData.dat_8006e48c != 0) {
        streamingData.musicEnabled = 1;
        func_8004FA24();
        return 1;
    } 
    
    if (cdState.isReading != 0) {
        if (cdState.readTime < cdState.maxReadTime) {
            return 1;
        }

        modeFlags = 0x80;
        func_8005DB1C();
        
        // Set the mode to double speed?
        func_8005E0BC(0xE, &modeFlags, 0);
        func_8005DB08(&func_80050504);

        // Wait for the CD subsystem to be ready after the reinitialization
        CdlComplete = 2;
        while (func_8005E074(1, 0) != CdlComplete) {
        }

        func_8005E0BC(2, (void *) &cdState.readLoc, 0);

        cdState.readTime = 0; // Reset the disc read time
        
        // Start the read
        func_8005D96C(cdState.size, cdState.outBuf, 0x80);

        return 1;
    }

    return func_8005E074(1, 0) != 2;
}

/**
 * ???() - func_80050504() - TECHNICALLY MATCHING
 * Some delay slot bullshit was resolved using an empty asm call, this will need correction later
 * Looks equivalent to CDReadDone from Spyro 1
 * https://decomp.me/scratch/Dt2wl
 */
void func_80050504(unsigned char arg0) {
    if (streamingData.dat_8006e480 != 0) {
        if (arg0 == 2) {
            streamingData.dat_8006e480 = 0;
            __asm__ volatile(""); // needed to match, but clearly they didn't do this
            return;
        }
        func_8005E0BC(2, &streamingData.dat_8006e478, 0);
        streamingData.dat_8006e484 = 0;
        func_8005D96C(streamingData.dat_8006e474, streamingData.dat_8006e47c, 0x80);
    }
}

/**
 * ???() - func_80050578()
 * Awful, but probably like func_80016500 from S1
 * S1 CD struct seems similar to the start of StreamingData too
 * Looks equivalent to CDLoadSync from Spyro 1
 * https://decomp.me/scratch/75XlZ
 */
INCLUDE_ASM("asm/nonmatchings/str", func_80050578);

/**
 * ???() - func_80050680()
 * Probably difficult
 * Looks equivalent to CDLoadAsync from Spyro 1
 * https://decomp.me/scratch/iHBib
 */
INCLUDE_ASM("asm/nonmatchings/str", func_80050680);

/**
 * FindMobyDialogue() - func_8005077C() - MATCHING
 * https://decomp.me/scratch/05p9Q
 */
int func_8005077C(SpeechProps* tag) {
    int speechStart;
    int i;
    SpeechData* speechData;

    speechData = ovlHeader.speechData[language];
    if (speechData != 0) {
        i = 0;
        while (speechData[i].typ != 0xFF) {
            if ((speechData[i].typ == tag->typ) && (speechData[i].msg == tag->nextMsg)) {
                speechStart = speechLba + speechData[i].offset;
                func_8004F9C0(speechStart, speechStart + speechData[i].len, speechData[i].channel);
                D_8006C674 = i;
                return 1;
            }
            i++;
        }
    }
    return 0;
}
