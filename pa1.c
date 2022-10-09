//---------------------------------------------------------------
//
//  4190.308 Computer Architecture (Fall 2022)
//
//  Project #1:
//
//  September 6, 2022
//
//  Seongyeop Jeong (seongyeop.jeong@snu.ac.kr)
//  Jaehoon Shim (mattjs@snu.ac.kr)
//  IlKueon Kang (kangilkueon@snu.ac.kr)
//  Wookje Han (gksdnrwp@snu.ac.kr)
//  Jinsol Park (jinsolpark@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//---------------------------------------------------------------

typedef unsigned char u8;

int numberOfBits(u8 num) {
    if(num == 0) {
        return 0;
    }
    if(num == 1) {
        return 1;
    }
    if(num < 4) {
        return 2;
    }
    if(num < 8) {
        return 3;
    }
    if(num < 16){
        return 4;
    }
    if(num < 32) {
        return 5;
    }
    if(num < 64) {
        return 6;
    }
    if(num < 128) {
        return 7;
    }
    return 8;
}

u8 getFilter(const u8* src, int width, int j) {
    u8 left_up = j%width > 0 && j> width-1 ? *(src+j-width-1) : 0;
    u8 up = j > width-1 ? *(src+j-width) : 0;
    u8 left = j%width > 0 ? *(src+j-1) : 0;
    int avg =(up + left + left_up);
    if(j%width > 0 && j> width-1){
     avg = avg/3;
    }
    u8 filter = *(src+j) - avg;
    filter = filter | filter+256;
    return filter;
}

/* TODO: Implement this function */
int encode(const u8* src, int width, int height, u8* result)
{
    int i,j = 0;

    int lengthWritten = 0;//update whenever you write any bit to result; It needs to get the last u8 index to write
    for(i = 0; i < height; i++) { //Iterate one time for each row

        u8 Base = 255;
        u8 NoB = 0;
        //getting Base (min{Filter})
        for(j = i*width; j < i*width+width; j++) {
            u8 filter = getFilter(src, width, j);
            if(Base > filter) {
                Base = filter;
            }
        }
        // add Base to Result and update lengthWritten
        int shiftRight = lengthWritten%8;
        u8 temp1 = Base >> shiftRight;
        u8 temp2;
        *(result+lengthWritten/8) = 255 << (8-shiftRight) & *(result + lengthWritten/8);
         if(shiftRight>0){
            int shiftLeft = 8 - shiftRight;
            temp2 = Base << shiftLeft;
            lengthWritten+= shiftLeft;
            *(result+lengthWritten/8) = temp2; // shift and check;
            lengthWritten+=shiftRight;
        }else {
            lengthWritten+=8;
        }


        for(j = i*width; j < i*width+width; j++) {
            u8 filter = getFilter(src, width, j);
            u8 delta = filter - Base;
            if(NoB < numberOfBits(delta)){
                NoB = numberOfBits(delta);
            }
        }

        // add NoB to Result
        u8 shouldBeWritten = NoB << 4;
        shiftRight = lengthWritten % 8;
        temp1 = shouldBeWritten >> shiftRight;
        *(result+lengthWritten/8) = 255 << (8-shiftRight) & *(result + lengthWritten/8); //setting to zero
        *(result+lengthWritten / 8) = temp1 | *(result+lengthWritten / 8);

        if (shiftRight > 4) {
            int canWriteBits = 8 - shiftRight;
            lengthWritten+= canWriteBits;
            temp2 = shouldBeWritten << canWriteBits;
            *(result+lengthWritten/8) = temp2;
            lengthWritten+= 4-canWriteBits;
        }else {
            lengthWritten+=4;
        }
        if(NoB == 0) {
            continue;
        }

        for(j = i*width; j < i*width+width; j++) {

            u8 filter = getFilter(src, width, j);
            u8 delta = filter - Base;

            shouldBeWritten = delta << (8-NoB);
            shiftRight = lengthWritten % 8;
            temp1 = shouldBeWritten >> shiftRight;
            *(result+lengthWritten/8) = 255 << (8-shiftRight) & *(result + lengthWritten/8);
            *(result+lengthWritten / 8) = temp1 | *(result+lengthWritten / 8);

            if (shiftRight > 8 - NoB) {
                int canWriteBits = 8 - shiftRight;
                lengthWritten+= canWriteBits;
                temp2 = shouldBeWritten << canWriteBits;
                *(result+lengthWritten/8) = temp2;
                lengthWritten+= NoB-canWriteBits;
            }else {
                lengthWritten+=NoB;
            }


        }

    }
    if(lengthWritten%8) {
        lengthWritten+= 8;
    }

  return lengthWritten/8;
}


