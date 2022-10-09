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

#include <stdio.h>
#include <string.h>

#include "pa1.h"

void print_ans(const u8* buf, const int buflen)
{
  for (int i = 0; i < buflen; i++) {
    if (i % 10 == 0 && i != 0) 
      printf("\n");
    if (buf[i] == 0) {
      printf("0x00, ");
      continue;
    }
    printf("0x%02x, ", buf[i]);
  }
  printf("\n");
}

void print_buffer(const u8* buf, const int height, const int width)
{
  for (int i = 0; i < height * width; i++) {
    if (i % width == 0 && i != 0)
      printf("\n");
    if (buf[i] == 0) {
      printf("0x00, ");
      continue;
    }
    printf("0x%02x, ", buf[i]);
  }
  printf("\n");
}

int __test_routine(const int num) {
  u8 dst[LEN_DST + LEN_GUARD] = {0,};

  *(unsigned long *)(dst + LEN_DST) = GUARD_WORD;

  printf("-------- Test #%d Encoding\n", num);

  int len = encode(tc[num].input, tc[num].input_width, tc[num].input_height, dst);

  printf("[Input] height (bytes): %d, width (bytes): %d\n", tc[num].input_height, tc[num].input_width);
  print_buffer(tc[num].input, tc[num].input_height, tc[num].input_width);

  printf("[Encode] length (bytes): %d\n", len);
  print_ans(dst, len);

  printf("[Answer] length (bytes): %d\n", tc[num].ans_len);
  print_ans(tc[num].ans, tc[num].ans_len);

  if (*(unsigned long *)(dst + LEN_DST) != GUARD_WORD)
    return 1;
  else if (len == -1 && tc[num].ans_len == 0)
    return 0; // no more test for dstlen overflow
  else if (len != tc[num].ans_len)
    return 2;
  else if (memcmp(dst, tc[num].ans, tc[num].ans_len) != 0)
    return 3;

  return 0;
}

int test_routine(const int num)
{
  int ret = __test_routine(num);

  if (!ret)
    printf("-------- ENCODING CORRECT!\n\n");
  else
    printf("-------- ENCODING WRONG! %d\n\n", ret);

  return !!ret;
}

int main() {
  int ret = 0;
  for (int i = 0; i < (sizeof tc / sizeof(testcase)); i++) {
    ret += test_routine(i);
  }
  return ret;
}

