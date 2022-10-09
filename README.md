# 4190.308 Computer Architecture (Fall 2022)

# Project #1: Simplified Image Compression

### Due: 11:59PM, September 18 (Sunday)


## Introduction

In this project, you need to perform a simplified image compression on the given grayscale image in memory. The purpose of this project is to make you familiar with the binary representation of integers and the bit-level operations supported in the C programming language. Another goal is to make your Linux or MacOS development environment ready and to get familiar with our project submission server.

## Background

### Grayscale Image

A pixel in a grayscale image represents only an amount of light, ranging from black at the weakest intensity to white at the strongest. Today, grayscale images intended for visual display are commonly quantized to unsigned integers with 8 bits per pixel. This pixel depth allows 256 different intensities from black (value 0) to white (value 255). This also simplifies computation as each pixel sample can be accessed individually as one full byte. 

### PNG Filtering

The PNG file format supports a precompression step called filtering. Filtering is a method of reversibly transforming the image data so that the main compression engine can operate more efficiently. As a simple example, suppose that the byte sequence increases uniformly from 1 to 255. Because there is no repetition in the sequence, it is either very poor or not compressed at all. However, a minor modification of the sequence (i.e., leaving the first byte alone but replacing each subsequent byte with the difference from the previous byte) transforms the sequence into a highly compressible set of 255 identical bytes, each having the value 1. In this project, we will use a simplified _Paeth filtering_ algorithm in the PNG format. For more details on PNG filtering, please refer to https://www.w3.org/TR/PNG-Filters.html

## Our Simplified Image Compression

Our simplified image compression scheme consists of two phases. In the first phase, we apply a simplified Paeth filtering algorithm to reduce the range of pixel values. In the second phase, we encode those values in a more compact binary representation.


### Phase 1: Simplified Paeth Filtering

The basic idea behind the Paeth filtering is to record only the difference from the neighboring pixel values because the value of a pixel changes gradually in most cases. Our simplified Paeth filtering algorithm works as follows. 

1. First, find the average of the three negiboring pixels in left, upper, and upper-left positions. When the neighboring pixel does not exist, exclude it from the calculation. 
Let us assume that `S[H][W]` represents an array of the pixel values of the input image where `H` indicates the number of rows and `W` the number of columns. The following shows how to get the average value `Avg[i][i]` for `S[i][j]`.

```
Avg[i][j] = 0                                            if i == 0 and j == 0,
            S[i-1][j]                                    if i != 0 and j == 0,
            S[i][j-1]                                    if i == 0 and j != 0,
            (S[i][j-1] + S[i-1][j] + S[i-1][j-1]) / 3    otherwise
            where 0 <= i < H and 0 <= j < W
```

Let's see an example. The following figure shows a 3x4 grayscale image where each pixel value (in decimal) is shown in the corresponding pixel.

<img src="https://github.com/snu-csl/ca-pa1/blob/main/sample.png?raw=true" alt="sample image" style="width:200px;">

```
Original image S[3][4]:
    0    0    0    0
   50   75  100  120
   75  100  120    0
```
```
Avg[3][4]:
    0    0    0    0
    0   16   25   33
   50   66   91  113
``` 

2. Get the filtered value by computing the difference between the actual pixel value and the average value, i.e. `S[i][j] - Avg[i][j]`. Note that the difference can be a negative value when `S[i][j] < Avg[i][j]`. In this case, we add 256 to `S[i][j]` before subtracting `Avg[i][j]` to ensure that all filtered values are positive. To summarize, the filtered value `Filter[i][j]` can be obtained as follows:

```
Filter[i][j] = S[i][j] - Avg[i][j]          if S[i][j] >= Avg[i][j],
               S[i][j] + 256 - Avg[i][j]    otherwise
```

In the previous example, `Filter[3][4]` can be obtained as follows.

```
Filter[3][4]:
    0    0    0    0    
   50   59   75   87   
   25   34   29  143   
``` 

Note that the original pixel value 0 is smaller that the average value 113 in the last pixel at `S[2][3]`. In this case, the difference should be -113 so that we can restore the original value using the sum of the average value and the filtered value such that 113 + (-113) = 0. However, we have added 256 to 0 and used the value 0 + 256 - 113 = 143 as the filtered value to make it positive. Even though this looks strange, we have no problem in restoring the original pixel value by obtaining only the lower 8 bits of the sum: i.e., (113 + 143) % 256 = 0. 

### Phase 2: Encoding Filtered Values

Once we get the filtered values, we encode those values as compact as possible. Since the filtered values usually have a smaller range, we can encode them using the smaller number of bits compared to the original pixel values. In order to encode the filtered values, we use the base-delta encoding scheme for each row.

3. Find the minimum and the maximum filtered values for each row. The minimum value will be used as the base value of the row and the deltas from the base value are calculated for each pixel in the given row. The following shows the base values and the deltas for each row in our example.

```
Filter[3][4]:                         
    0    0    0    0     min:   0, max:   0   
   50   59   75   87     min:  50, max:  87
   25   34   29  143     min:  25, max: 143

Delta[3][4]:
   +0   +0   +0   +0     base(0): 0
   +0   +9  +25  +37     base(1): 50
   +0   +9  +4  +118     base(2): 25
``` 

4. Find the number of bits needed for representing the deltas in an unsigned integer format in each row. It can be calculated from the maximum delta value for each row. In the previous example, the second and the third row has the maximum delta value 37 and 118, respectively. For the second row, those delta values can be encoded using just 6 bits, while we need 7 bits for the third row. For the first row, all the delta values are same and we don't have to use additional bits to encode them. 

For the row `i`, the number of bits `n(i)` needed to encode delta values with unsigned integers can be obtained as follows:
```
n(i) = 0        if max(Delta[i]) == 0,
       1        else if max(Delta[i]) == 1,
       2        else if max(Delta[i]) < 4,
       3        else if max(Delta[i]) < 8,
       4        else if max(Delta[i]) < 16,
       5        else if max(Delta[i]) < 32,
       6        else if max(Delta[i]) < 64,
       7        else if max(Delta[i]) < 128,
       8        otherwise
```

In our example, `n(i)` is calculated as shown below:
```
Delta[3][4]:
   +0   +0   +0   +0     n(0) = 0
   +0   +9  +25  +37     n(1) = 6
   +0   +9  +4  +118     n(2) = 7
``` 

5. Now we encode each row, one row at a time. The row `i` is encoded as follows.

```
|  Base value  | # of bits | 0-th Delta Value | ... | (W-1)-th Delta Value |
|  base(i)     | n(i)      | Delta[i][0]      |     | Delta[i][W-1]        |
|--------------|-----------|------------------|-----|----------------------|
|   (8 bits)   | (4 bits)  |   (n(i) bits)    | ... |     (n(i) bits)      |
```

There is a special case; when `n(i)` is zero, it means that all filtered values in the given row `i` are identical. In this case, encoding the delta values can be skipped. In other words, encoding the row `i` such that `n(i) == 0` is simplified as follows.

```
|  Base value  | # of bits | 
|  base(i)     | n(i)      | 
|--------------|-----------|
|   (8 bits)   | 0000      |
```

For our example, each row is encoded as follows.
```
       base(i)  n(i) 
Row 0: 00000000 0000
Row 1: 00110010 0110 000000  001001  011001  100101
Row 2: 00011001 0111 0000000 0001001 0000100 1110110 
```

6. If the total number of output bits is not a multiple of 8 after encoding, pad 0's until it becomes a multiple of 8. This is because byte is the smallest unit that can be stored in memory.

Luckily, the total number of encoded bits in our example is already a multiple of 8, so we don't need any padding. The final encoded bytes can be written in hexadecimal numbers as follows:

```
Output: 0x00 0x03 0x26 0x00 0x96 0x65 0x19 0x70 0x02 0x42 0x76 (11 bytes)
```

## Problem specification

Write the C function named `encode()` that encodes the input binary data using our simplified image compression scheme. The prototype of `encode()` is as follows:

```
typedef unsigned char u8;
int encode(const u8* src, const int width, const int height, u8* dst);
```

The first argument `src` points to the memory address of the input data. The width and height of the input data (in bytes) are specified in the second argument `width` and third `height`, respectively. The encoded result should be stored in the memory that starts from the address pointed to by `dst`.

The function `encode()` returns the actual length of the output in bytes including the encoded data and padded bits. When `width` or `height` is zero, `encode()` returns zero. You can safely assume that a sufficient amount of memory is already allocated for the buffer designated by `dst`. However, the contents of the buffer after the encoded output should not be corrupted in any case.


## Skeleton code

We provide you with the skeleton code for this project. It can be downloaded from Github at https://github.com/snu-csl/ca-pa1/. If you don't have the `git` utility, you need to install it first. You can install the `git` utility on Ubuntu by running the following command:

```
$ sudo apt install git
```

For MacOS, install the Xcode command line tools which come with `git`.

To download and build the skeleton code, please follow these steps:

```
$ git clone https://github.com/snu-csl/ca-pa1.git
$ cd ca-pa1
$ make
gcc -g -O2 -Wall    -c -o main.o main.c
gcc -g -O2 -Wall    -c -o pa1.o pa1.c
gcc -o pa1 main.o pa1.o
```

The result of a sample run looks like this:
```
$ ./pa1
-------- Test #0 Encoding
[Input] width (bytes): 4, height (bytes): 3
0x00, 0x00, 0x00, 0x00,
0x32, 0x4b, 0x64, 0x78,
0x4b, 0x64, 0x78, 0x00,
[Encode] length (bytes): 0

[Answer] length (bytes): 11
0x00, 0x03, 0x26, 0x00, 0x96, 0x65, 0x19, 0x70, 0x02, 0x42,
0x76,
-------- ENCODING WRONG! 2

-------- Test #1 Encoding
[Input] width (bytes): 10, height (bytes): 5
0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
[Encode] length (bytes): 0

[Answer] length (bytes): 45
0x00, 0x8f, 0xf0, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0x01, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0xc0, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x80, 0x1a, 0xb0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00,
-------- ENCODING WRONG! 2

(... more test cases below ...)
```

## Restrictions

* You are not allowed to use any array even in the comment lines. Any source file that contains the symbol `[` or `]` will be rejected by the server. 

* Do not include any header file in the `pa1.c` file. You are not allowed to use any library functions (including `printf()`) inside the `pa1.c` file. 

* Your solution should finish within a reasonable time. If your code does not finish within a predefined threshold (e.g., 5 sec), it will be killed.


## Hand in instructions

* In order to submit your solution, you need to register an account to the submission server at https://sys.snu.ac.kr
  * You must enter your real name & student ID
  * Wait for an approval from the TA
* Note that the submission server is only accessible inside the SNU campus network. If you want off-campus access to the submission server, please submit your IP through Google Form (https://forms.gle/rbWD2ZV2mAxRT1Ar5)
* Upload only the `pa1.c` file to the submission server

## Logistics

* You will work on this project alone.
* Only the upload submitted before the deadline will receive the full credit. 25% of the credit will be deducted for every single day delay.
* __You can use up to 4 _slip days_ during this semester__. If your submission is delayed by 1 day and if you decided to use 1 slip day, there will be no penalty. In this case, you should explicitly declare the number of slip days you want to use in the QnA board of the submission server __after__ each submission. Saving the slip days for later projects is highly recommended!
* Any attempt to copy others' work will result in heavy penalty (for both the copier and the originator). Don't take a risk.

Have fun!

[Jin-Soo Kim](mailto:jinsoo.kim_AT_snu.ac.kr)  
[Systems Software and Architecture Laboratory](http://csl.snu.ac.kr)  
[Dept. of Computer Science and Engineering](http://cse.snu.ac.kr)  
[Seoul National University](http://www.snu.ac.kr)
