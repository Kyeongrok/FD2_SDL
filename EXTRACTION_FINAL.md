# FDOTHER.DAT 이미지 추출 최종 보고서

## 문제 수정

### 1. 이미지 상하 반전
**해결 방법**: 추출 후 이미지를 수직으로 뒤집어야 함

### 2. RLE 압축 형식
**최종 확인된 형식**:
| 타입 | 비트 패턴 | 설명 |
|------|--------|------|
| 00 | 00xxxxxx | 다음 바이트로 num 바이트 채우기 |
| 01 | 01xxxxxx | num 바이트 복사 (리터럴 복사) |
| 10 | 10xxxxxx | 다음 바이트로 num 바이트 채우기 |
| 11 | 11xxxxxx | 건너뛰기 (포인터만 전진, remaining-=num) |

num = (code & 0x3F) + 1

## 추출 결과

| 리소스 | 크기 | 타입 | 상태 |
|------|------|------|------|
| 11 | 53587 | RLE | ✓ 완전한 게임 배경 |
| 15 | 64004 | RAW | ✓ 타이틀 화면 |
| 55 | 64004 | RAW | ✓ 배경 이미지 |
| 56 | 13609 | RLE | ✓ 내용 있음 |
| 59 | 3278 | RLE | 부분 (오버레이?) |
| 60 | 2849 | RLE | 부분 (오버레이?) |
| 61 | 54727 | RLE | ✓ 완전한 게임 배경 |
| 62 | 43434 | RLE | ✓ 내용 있음 |
| 74 | 6585 | RLE | 부분 (파란색 영역) |
| 75 | 37776 | RLE | ✓ 내용 있음 |
| 97 | 39358 | RLE | ✓ 내용 있음 |
| 100 | 15746 | RLE | 부분 (오버레이?) |

## 출력 파일

- `images/res_*_v6.png` - 최종 추출된 PNG 이미지
- `src/fd2_rle.c` - 업데이트된 RLE 압축 해제 함수
- `src/fdother_extract.c` - 완전한 추출 도구

## RLE 압축 해제 알고리즘

```c
void decompress_rle(byte* src, int src_size, byte* dst, int stride) {
    word count = *(word*)src;      // 每行字节数
    word num_rows = *(word*)(src + 2);  // 行数
    
    src += 4;
    int pos = 0;
    
    for (int row = 0; row < num_rows; row++) {
        int remaining = count;
        
        while (remaining > 0 && pos < src_size) {
            byte code = src[pos++];
            int num = (code & 0x3F) + 1;
            if (num > remaining) num = remaining;
            
            int type = (code >> 6) & 0x3;
            
            switch(type) {
                case 0: // 00: fill
                case 2: // 10: fill
                    byte fill = src[pos++];
                    memset(dst, fill, num);
                    dst += num;
                    break;
                case 1: // 01: literal
                    memcpy(dst, src + pos, num);
                    pos += num;
                    dst += num;
                    break;
                case 3: // 11: skip
                    dst += num;
                    break;
            }
            remaining -= num;
        }
        dst += stride - count;
    }
}
```
