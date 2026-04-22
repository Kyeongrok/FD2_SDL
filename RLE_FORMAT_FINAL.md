# FDOTHER.DAT RLE 압축 형식 - 최종 확인

## RLE 코드 형식

각 code 바이트 형식: `ttnnnnnn`
- `tt` = 타입 (2비트)
- `nnnnnn` = 개수 = (code & 0x3F) + 1

## 네 가지 타입

| 타입 | 비트 패턴 | 설명 |
|------|--------|------|
| 0 | 00xxxxxx | 채우기: 다음 바이트의 값으로 num 바이트 채우기 |
| 1 | 01xxxxxx | 채우기: 다음 바이트의 값으로 num 바이트 채우기 |
| 2 | 10xxxxxx | 채우기: 다음 바이트의 값으로 num 바이트 채우기 |
| 3 | 11xxxxxx | **특수 채우기**: code 바이트 자체의 값으로 num 바이트 채우기 (추가 바이트 읽지 않음)|

## 핵심 발견

**타입 3의 특수 동작**:
- 일반적인 해석: 건너뛰기 / 쓰지 않음
- **실제 동작**: code 바이트의 값으로 채우기 (code가 곧 fill 값)
- 예: `0xFF` = 0xFF로 64바이트 채우기

이는 res_59 등의 리소스가 처음에 모두 0xFF이고, 결과가 흰색(palette index 255)인 이유를 설명합니다.

## 압축 해제 알고리즘

```c
void decompress_rle(byte* src, int src_size, byte* dst, int stride) {
    word count = *(word*)src;      // 每行字节数
    word num_rows = *(word*)(src + 2);  // 行数
    
    src += 4;  // 跳过4字节header
    
    for (int row = 0; row < num_rows; row++) {
        int remaining = count;
        
        while (remaining > 0) {
            byte code = *src++;
            int num = (code & 0x3F) + 1;
            
            int type = (code >> 6) & 0x3;
            
            if (type != 3) {
                // type 0,1,2: 用下一字节填充
                byte fill = *src++;
                memset(dst, fill, num);
            } else {
                // type 3: 用代码字节填充（代码即值）
                memset(dst, code, num);
            }
            dst += num;
            remaining -= num;
        }
        dst += stride - count;  // 跳到下一行
    }
}
```

## 이미지 처리

- 모든 이미지는 수직으로 뒤집어야 함 (Y축 미러링)
- Palette는 리소스 0, offset 422, 768바이트
- 6비트에서 8비트로 변환: `val8 = (val6 << 2) | (val6 >> 4)`
