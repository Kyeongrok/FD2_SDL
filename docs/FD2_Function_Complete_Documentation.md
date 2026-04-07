# FD2.EXE 逆向函数完整说明文档

本文档包含 FD2.EXE 中 **1228** 个逆向函数的完整列表，基于 IDA Pro 反汇编分析和 decompile 工具导出的伪代码。

---

## 统计信息

| 项目 | 数量 |
|------|------|
| 总函数数 | 1228 |
| 有函数名的函数 | 1228 |
| 有调用者信息的函数 | 1150 |
| 有被调用者信息的函数 | 862 |
| 总调用引用数 | 4942 |
| 总被调用引用数 | 4884 |

---

## 函数列表

按地址排序的完整函数列表：

| 地址 | 函数名 | 调用者数 | 被调用数 | 原型 |
|------|--------|----------|----------|------|
| 0x10000 | sub_10000 | 5 | 0 | void |
| 0x10010 | sub_10010 | 2 | 10 | void |
| 0x10620 | sub_10620 | 17 | 0 | int |
| 0x10652 | sub_10652 | 3 | 2 | int |
| 0x1088D | sub_1088D | 3 | 14 | int __cdecl(int n13) |
| 0x10B4E | sub_10B4E | 46 | 7 | int __fastcall |
| 0x10C50 | sub_10C50 | 1 | 11 | int __fastcall |
| 0x11019 | sub_11019 | 8 | 5 | int __fastcall |
| 0x111BA | sub_111BA | 38 | 8 | _BYTE* __fastcall |
| 0x112A5 | sub_112A5 | 22 | 6 | int __fastcall |
| 0x1145A | sub_1145A | 2 | 3 | int __fastcall |
| 0x11506 | sub_11506 | 1 | 2 | void __fastcall |
| 0x115B6 | sub_115B6 | 4 | 14 | void __fastcall |
| 0x117E7 | sub_117E7 | 1 | 106 | int __usercall |
| 0x11AA8 | sub_11AA8 | 1 | 5 | int __fastcall |
| 0x11B48 | sub_11B48 | 3 | 3 | int __fastcall |
| 0x11B9B | sub_11B9B | 3 | 3 | int __fastcall |
| 0x11BFA | sub_11BFA | 3 | 3 | int __fastcall |
| 0x11C59 | sub_11C59 | 3 | 3 | int __fastcall |
| 0x11CAC | sub_11CAC | 50 | 7 | int __fastcall |
| 0x11D40 | sub_11D40 | 19 | 2 | void __fastcall |
| 0x11DF2 | sub_11DF2 | 1 | 8 | int __fastcall |
| 0x11EB0 | sub_11EB0 | 50 | 2 | int __fastcall |
| 0x11EEE | sub_11EEE | 4 | 9 | int __fastcall |
| 0x122DC | sub_122DC | 2 | 2 | int __fastcall |
| 0x126F7 | sub_126F7 | 3 | 1 | int __fastcall |
| 0x127A9 | sub_127A9 | 6 | 1 | int __fastcall |
| 0x127E0 | sub_127E0 | 7 | 3 | void __fastcall |
| 0x1297D | sub_1297D | 5 | 1 | int __fastcall |
| 0x129EC | sub_129EC | 6 | 4 | int __fastcall |
| 0x12AC6 | sub_12AC6 | 1 | 8 | int __fastcall |
| 0x12C0D | sub_12C0D | 3 | 1 | int __fastcall |
| 0x12C60 | sub_12C60 | 2 | 5 | int __fastcall |
| 0x12CEA | sub_12CEA | 5 | 3 | int __fastcall |
| 0x12D7B | sub_12D7B | 19 | 2 | void __fastcall |
| 0x12DAC | sub_12DAC | 2 | 1 | int __fastcall |
| 0x12E38 | sub_12E38 | 3 | 1 | int __fastcall |
| 0x12EAA | sub_12EAA | 3 | 7 | int __fastcall |
| 0x1300D | sub_1300D | 1 | 9 | int __fastcall |
| 0x134E4 | sub_134E4 | 2 | 0 | void __fastcall |
| 0x135DD | sub_135DD | 51 | 3 | void __fastcall |
| 0x1366A | sub_1366A | 49 | 11 | int __fastcall |
| 0x1399C | sub_1399C | 2 | 6 | int __fastcall |
| 0x13A44 | sub_13A44 | 4 | 0 | void __fastcall |
| 0x13FD4 | sub_13FD4 | 5 | 7 | int __fastcall |
| 0x145CD | sub_145CD | 5 | 2 | void __fastcall |
| 0x14625 | sub_14625 | 4 | 1 | void __fastcall |
| 0x146A7 | sub_146A7 | 1 | 0 | void __fastcall |
| 0x146D1 | sub_146D1 | 2 | 2 | int __fastcall |
| 0x14742 | sub_14742 | 5 | 0 | int __fastcall |
| 0x14B16 | sub_14B16 | 2 | 2 | int __fastcall |
| 0x14EF0 | sub_14EF0 | 1 | 6 | void __fastcall |
| 0x1567E | sub_1567E | 3 | 3 | int __fastcall |
| 0x1598A | sub_1598A | 2 | 4 | int __fastcall |
| 0x15B77 | sub_15B77 | 1 | 9 | int __fastcall |
| 0x15DA2 | sub_15DA2 | 2 | 0 | void __fastcall |
| 0x15F84 | sub_15F84 | 2 | 16 | int __fastcall |
| 0x165AC | sub_165AC | 2 | 8 | int __fastcall |
| 0x168B6 | sub_168B6 | 2 | 9 | int __fastcall |
| 0x16B43 | sub_16B43 | 1 | 7 | int __fastcall |
| 0x16C57 | sub_16C57 | 3 | 10 | int __fastcall |
| 0x16E24 | sub_16E24 | 3 | 4 | int __fastcall |
| 0x16F55 | sub_16F55 | 2 | 15 | int __fastcall |
| 0x1728C | sub_1728C | 1 | 9 | int __fastcall |
| 0x1741C | sub_1741C | 5 | 7 | int __fastcall |
| 0x175A9 | sub_175A9 | 3 | 1 | void __fastcall |
| 0x176B4 | sub_176B4 | 4 | 6 | int __fastcall |
| 0x177FC | sub_177FC | 2 | 5 | int __fastcall |
| 0x179D5 | sub_179D5 | 3 | 5 | int __fastcall |
| 0x17AA9 | sub_17AA9 | 3 | 0 | void __fastcall |
| 0x17AED | sub_17AED | 6 | 7 | int __fastcall |
| 0x17E0B | sub_17E0B | 1 | 5 | int __fastcall |
| 0x17EEF | sub_17EEF | 4 | 3 | int __fastcall |
| 0x17FC0 | sub_17FC0 | 1 | 15 | int __fastcall |
| 0x182AD | sub_182AD | 1 | 1 | int __fastcall |
| 0x1839B | sub_1839B | 2 | 2 | void __fastcall |
| 0x184C0 | sub_184C0 | 1 | 11 | int __fastcall |
| 0x187D6 | sub_187D6 | 1 | 6 | int __fastcall |
| 0x18890 | sub_18890 | 3 | 12 | int __fastcall |
| 0x18B84 | sub_18B84 | 3 | 4 | int __fastcall |
| 0x18C6D | sub_18C6D | 1 | 6 | int __fastcall |
| 0x18D8C | sub_18D8C | 4 | 12 | int __fastcall |
| 0x190AC | sub_190AC | 1 | 26 | int __fastcall |
| 0x1956B | sub_1956B | 4 | 6 | int __fastcall |
| 0x196CB | sub_196CB | 3 | 2 | void __fastcall |
| 0x1974C | sub_1974C | 2 | 4 | int __fastcall |
| 0x197E5 | sub_197E5 | 4 | 6 | int __fastcall |
| 0x19DF7 | sub_19DF7 | 1 | 25 | int __fastcall |
| 0x1A7BD | sub_1A7BD | 5 | 1 | int __fastcall |
| 0x1A7F1 | sub_1A7F1 | 3 | 0 | void __fastcall |
| 0x1A813 | sub_1A813 | 4 | 1 | void __fastcall |
| 0x1A866 | sub_1A866 | 6 | 5 | int __fastcall |
| 0x1AA1D | sub_1AA1D | 1 | 13 | int __fastcall |
| 0x1ACF3 | sub_1ACF3 | 5 | 3 | int __fastcall |
| 0x1AEB1 | sub_1AEB1 | 2 | 2 | int __fastcall |
| 0x1AF1E | sub_1AF1E | 3 | 1 | void __fastcall |
| 0x1AF99 | sub_1AF99 | 2 | 3 | int __fastcall |
| 0x1B019 | sub_1B019 | 2 | 4 | int __fastcall |
| 0x1B0AD | sub_1B0AD | 2 | 4 | int __fastcall |
| 0x1B14B | sub_1B14B | 2 | 4 | int __fastcall |
| 0x1B1E7 | sub_1B1E7 | 5 | 7 | int __fastcall |
| 0x1B41D | sub_1B41D | 1 | 8 | int __fastcall |
| 0x1B5F1 | sub_1B5F1 | 2 | 1 | void __fastcall |
| 0x1B653 | sub_1B653 | 2 | 1 | void __fastcall |
| 0x1B6B7 | sub_1B6B7 | 2 | 1 | void __fastcall |
| 0x1B722 | sub_1B722 | 2 | 0 | void __fastcall |
| 0x1B750 | sub_1B750 | 11 | 3 | void __fastcall |
| 0x1B83D | sub_1B83D | 4 | 1 | void __fastcall |
| 0x1B8A6 | sub_1B8A6 | 3 | 0 | void __fastcall |
| 0x1B8E7 | sub_1B8E7 | 3 | 0 | void __fastcall |
| 0x1B932 | sub_1B932 | 3 | 2 | int __fastcall |
| 0x1B9DE | sub_1B9DE | 2 | 6 | int __fastcall |
| 0x1BB8C | sub_1BB8C | 2 | 1 | void __fastcall |
| 0x1BBDE | sub_1BBDE | 5 | 10 | int __fastcall |
| 0x1BFFE | sub_1BFFE | 3 | 5 | int __fastcall |
| 0x1C142 | sub_1C142 | 2 | 2 | int __fastcall |
| 0x1C1C3 | sub_1C1C3 | 2 | 2 | int __fastcall |
| 0x1C220 | sub_1C220 | 2 | 1 | void __fastcall |
| 0x1C269 | sub_1C269 | 3 | 2 | int __fastcall |
| 0x1C2DA | sub_1C2DA | 7 | 6 | int __fastcall |
| 0x1C4CC | sub_1C4CC | 6 | 8 | int __fastcall |
| 0x1C75E | sub_1C75E | 2 | 4 | int __fastcall |
| 0x1C81F | sub_1C81F | 2 | 5 | int __fastcall |
| 0x1C8ED | sub_1C8ED | 3 | 0 | void __fastcall |
| 0x1C916 | sub_1C916 | 2 | 5 | int __fastcall |
| 0x1C9DD | sub_1C9DD | 2 | 4 | int __fastcall |
| 0x1CA89 | sub_1CA89 | 2 | 0 | void __fastcall |
| 0x1CAC7 | sub_1CAC7 | 4 | 6 | int __fastcall |
| 0x1CB94 | sub_1CB94 | 2 | 8 | int __fastcall |
| 0x1CD17 | sub_1CD17 | 5 | 7 | int __fastcall |
| 0x1CEED | sub_1CEED | 2 | 5 | int __fastcall |
| 0x1CFF0 | sub_1CFF0 | 4 | 14 | int __fastcall |
| 0x1D4CB | sub_1D4CB | 5 | 1 | int __fastcall |
| 0x1D4F6 | sub_1D4F6 | 5 | 0 | void __fastcall |
| 0x1D51D | sub_1D51D | 3 | 6 | int __fastcall |
| 0x1D6C8 | sub_1D6C8 | 2 | 5 | int __fastcall |
| 0x1D79C | sub_1D79C | 2 | 2 | void __fastcall |
| 0x1D80B | sub_1D80B | 2 | 4 | int __fastcall |
| 0x1D8BA | sub_1D8BA | 2 | 7 | int __fastcall |
| 0x1DA16 | sub_1DA16 | 2 | 7 | int __fastcall |
| 0x1DB65 | sub_1DB65 | 7 | 9 | int __fastcall |
| 0x1DEBE | sub_1DEBE | 2 | 4 | int __fastcall |
| 0x1DF58 | sub_1DF58 | 5 | 6 | int __fastcall |
| 0x1E0DB | sub_1E0DB | 1 | 7 | int __fastcall |
| 0x1E1DC | sub_1E1DC | 1 | 6 | int __fastcall |
| 0x1E292 | sub_1E292 | 3 | 5 | int __fastcall |
| 0x1E529 | sub_1E529 | 1 | 7 | int __fastcall |
| 0x1E5C0 | sub_1E5C0 | 3 | 4 | int __fastcall |
| 0x1E611 | sub_1E611 | 1 | 5 | int __fastcall |
| 0x1E739 | sub_1E739 | 1 | 6 | int __fastcall |
| 0x1E7F6 | sub_1E7F6 | 1 | 5 | int __fastcall |
| 0x1E856 | sub_1E856 | 1 | 5 | int __fastcall |
| 0x1E98C | sub_1E98C | 1 | 6 | int __fastcall |
| 0x1EB05 | sub_1EB05 | 1 | 5 | int __fastcall |
| 0x1EC2A | sub_1EC2A | 1 | 5 | int __fastcall |
| 0x1ECC7 | sub_1ECC7 | 1 | 4 | int __fastcall |
| 0x1F04A | sub_1F04A | 1 | 5 | int __fastcall |
| 0x1F0DC | sub_1F0DC | 1 | 5 | int __fastcall |
| 0x1F183 | sub_1F183 | 4 | 1 | int __fastcall |
| 0x1F1CC | sub_1F1CC | 5 | 3 | void __fastcall |
| 0x1F30A | sub_1F30A | 6 | 3 | void __fastcall |
| 0x1F42D | sub_1F42D | 4 | 6 | int __fastcall |
| 0x1F525 | sub_1F525 | 5 | 3 | void __fastcall |
| 0x1F558 | sub_1F558 | 2 | 2 | int __fastcall |
| 0x1F6EF | sub_1F6EF | 1 | 6 | int __fastcall |
| 0x1F73F | sub_1F73F | 5 | 4 | int __fastcall |
| 0x1F81E | sub_1F81E | 6 | 3 | void __fastcall |
| 0x1F882 | sub_1F882 | 5 | 3 | void __fastcall |
| 0x1F894 | sub_1F894 | 7 | 3 | void __fastcall |
| 0x1FF79 | sub_1FF79 | 1 | 5 | int __fastcall |
| 0x2000A | sub_2000A | 5 | 8 | int __fastcall |
| 0x203BD | sub_203BD | 1 | 4 | int __fastcall |
| 0x205B4 | sub_205B4 | 5 | 6 | int __fastcall |
| 0x205BE | sub_205BE | 2 | 2 | void __fastcall |
| 0x205DA | sub_205DA | 1 | 5 | int __fastcall |
| 0x206C5 | sub_206C5 | 5 | 5 | int __fastcall |
| 0x2073D | sub_2073D | 5 | 4 | int __fastcall |
| 0x2084A | sub_2084A | 5 | 3 | int __fastcall |
| 0x208CF | sub_208CF | 5 | 3 | int __fastcall |
| 0x20A51 | sub_20A51 | 5 | 3 | int __fastcall |
| 0x20A87 | sub_20A87 | 5 | 3 | int __fastcall |
| 0x20AAF | sub_20AAF | 5 | 3 | int __fastcall |
| 0x20B14 | sub_20B14 | 5 | 3 | int __fastcall |
| 0x20B3C | sub_20B3C | 5 | 3 | int __fastcall |
| 0x20B72 | sub_20B72 | 5 | 3 | int __fastcall |
| 0x20BF5 | sub_20BF5 | 5 | 3 | int __fastcall |
| 0x20C6F | sub_20C6F | 4 | 4 | int __fastcall |
| 0x2111A | sub_2111A | 4 | 5 | int __fastcall |
| 0x211A4 | sub_211A4 | 1 | 5 | int __fastcall |
| 0x2134B | sub_2134B | 1 | 6 | int __fastcall |
| 0x2137D | sub_2137D | 1 | 4 | int __fastcall |
| 0x213B7 | sub_213B7 | 1 | 4 | int __fastcall |
| 0x2147B | sub_2147B | 1 | 4 | int __fastcall |
| 0x214AD | sub_214AD | 2 | 3 | int __fastcall |
| 0x2189A | sub_2189A | 5 | 6 | int __fastcall |
| 0x219AD | sub_219AD | 1 | 8 | int __fastcall |
| 0x21B18 | sub_21B18 | 1 | 7 | int __fastcall |
| 0x21DB2 | sub_21DB2 | 1 | 5 | int __fastcall |
| 0x21EB1 | sub_21EB1 | 5 | 6 | int __fastcall |
| 0x226EA | sub_226EA | 1 | 4 | int __fastcall |
| 0x2282F | sub_2282F | 1 | 4 | int __fastcall |
| 0x22A85 | sub_22A85 | 1 | 4 | int __fastcall |
| 0x22AA8 | sub_22AA8 | 1 | 4 | int __fastcall |
| 0x22AF6 | sub_22AF6 | 2 | 3 | int __fastcall |
| 0x22BC6 | sub_22BC6 | 1 | 3 | int __fastcall |
| 0x22BE1 | sub_22BE1 | 1 | 2 | int __fastcall |
| 0x22C04 | sub_22C04 | 2 | 2 | int __fastcall |
| 0x22CBF | sub_22CBF | 1 | 2 | int __fastcall |
| 0x22CDA | sub_22CDA | 1 | 2 | int __fastcall |
| 0x22D1B | sub_22D1B | 2 | 2 | int __fastcall |
| 0x22E41 | sub_22E41 | 1 | 2 | int __fastcall |
| 0x22E5C | sub_22E5C | 3 | 3 | int __fastcall |
| 0x22EF6 | sub_22EF6 | 1 | 2 | int __fastcall |
| 0x22F37 | sub_22F37 | 50 | 8 | int __fastcall |
| 0x230F2 | sub_230F2 | 3 | 3 | int __fastcall |
| 0x231BC | sub_231BC | 1 | 4 | int __fastcall |
| 0x231F9 | sub_231F9 | 3 | 3 | int __fastcall |
| 0x232E8 | sub_232E8 | 3 | 3 | int __fastcall |
| 0x233C6 | sub_233C6 | 3 | 4 | int __fastcall |
| 0x234BB | sub_234BB | 3 | 5 | void __fastcall |
| 0x235BC | sub_235BC | 49 | 8 | int __fastcall |
| 0x235F9 | sub_235F9 | 49 | 8 | int __fastcall |
| 0x237D5 | sub_237D5 | 49 | 9 | int __fastcall |
| 0x2389F | sub_2389F | 3 | 3 | int __fastcall |
| 0x238DC | sub_238DC | 49 | 8 | int __fastcall |
| 0x239BD | sub_239BD | 3 | 3 | int __fastcall |
| 0x23A0A | sub_23A0A | 49 | 8 | int __fastcall |
| 0x23B5F | sub_23B5F | 50 | 8 | int __fastcall |
| 0x23CD5 | sub_23CD5 | 49 | 8 | int __fastcall |
| 0x23E39 | sub_23E39 | 1 | 4 | int __fastcall |
| 0x23E74 | sub_23E74 | 50 | 8 | int __fastcall |
| 0x240FA | sub_240FA | 49 | 8 | int __fastcall |
| 0x24B14 | sub_24B14 | 1 | 4 | int __fastcall |
| 0x24B4D | sub_24B4D | 5 | 5 | int __fastcall |
| 0x24C1E | sub_24C1E | 6 | 5 | int __fastcall |
| 0x24D22 | sub_24D22 | 1 | 4 | int __fastcall |
| 0x24DF2 | sub_24DF2 | 50 | 8 | int __fastcall |
| 0x24E80 | sub_24E80 | 2 | 3 | int __fastcall |
| 0x250CC | sub_250CC | 2 | 3 | int __fastcall |
| 0x25A96 | sub_25A96 | 10 | 2 | int __fastcall |
| 0x25B45 | sub_25B45 | 1 | 4 | int __fastcall |
| 0x25BF4 | sub_25BF4 | 5 | 6 | int __fastcall |
| 0x25EBB | sub_25EBB | 5 | 5 | int __fastcall |
| 0x265EC | sub_265EC | 3 | 4 | int __fastcall |
| 0x2670E | sub_2670E | 3 | 4 | int __fastcall |
| 0x26A0D | sub_26A0D | 1 | 3 | int __fastcall |
| 0x26A7A | sub_26A7A | 1 | 3 | int __fastcall |
| 0x26B91 | sub_26B91 | 1 | 3 | int __fastcall |
| 0x26C9B | sub_26C9B | 1 | 3 | int __fastcall |
| 0x26CE4 | sub_26CE4 | 1 | 3 | int __fastcall |
| 0x26E38 | sub_26E38 | 1 | 3 | int __fastcall |
| 0x26EDA | sub_26EDA | 3 | 4 | int __fastcall |
| 0x272D0 | sub_272D0 | 1 | 3 | int __fastcall |
| 0x275E6 | sub_275E6 | 1 | 3 | int __fastcall |
| 0x278E7 | sub_278E7 | 1 | 3 | int __fastcall |
| 0x279BC | sub_279BC | 5 | 4 | int __fastcall |
| 0x27D33 | sub_27D33 | 1 | 3 | int __fastcall |
| 0x27F4A | sub_27F4A | 1 | 3 | int __fastcall |
| 0x2810B | sub_2810B | 1 | 3 | int __fastcall |
| 0x2825B | sub_2825B | 1 | 3 | int __fastcall |
| 0x2860A | sub_2860A | 1 | 3 | int __fastcall |
| 0x2872B | sub_2872B | 3 | 5 | int __fastcall |
| 0x28B41 | sub_28B41 | 1 | 4 | int __fastcall |
| 0x28CBD | sub_28CBD | 3 | 4 | int __fastcall |
| 0x28EFE | sub_28EFE | 3 | 4 | int __fastcall |
| 0x28F65 | sub_28F65 | 3 | 4 | int __fastcall |
| 0x2921A | sub_2921A | 1 | 3 | int __fastcall |
| 0x2968D | sub_2968D | 1 | 4 | int __fastcall |
| 0x2986F | sub_2986F | 2 | 4 | int __fastcall |
| 0x29AB2 | sub_29AB2 | 1 | 4 | int __fastcall |
| 0x29BCB | sub_29BCB | 1 | 4 | int __fastcall |
| 0x29DAA | sub_29DAA | 4 | 4 | int __fastcall |
| 0x2A07A | sub_2A07A | 1 | 3 | int __fastcall |
| 0x2A0C2 | sub_2A0C2 | 1 | 3 | int __fastcall |
| 0x2A29D | sub_2A29D | 1 | 4 | int __fastcall |
| 0x2A43E | sub_2A43E | 1 | 4 | int __fastcall |
| 0x2A694 | sub_2A694 | 1 | 3 | int __fastcall |
| 0x2A857 | sub_2A857 | 1 | 3 | int __fastcall |
| 0x2AA00 | sub_2AA00 | 2 | 4 | int __fastcall |
| 0x2AC7D | sub_2AC7D | 3 | 4 | int __fastcall |
| 0x2AE0E | sub_2AE0E | 1 | 3 | int __fastcall |
| 0x2AEDB | sub_2AEDB | 1 | 3 | int __fastcall |
| 0x2AF28 | sub_2AF28 | 2 | 4 | int __fastcall |
| 0x2B439 | sub_2B439 | 1 | 3 | int __fastcall |
| 0x2B4FB | sub_2B4FB | 1 | 3 | int __fastcall |
| 0x2B67F | sub_2B67F | 3 | 4 | int __fastcall |
| 0x2B749 | sub_2B749 | 1 | 3 | int __fastcall |
| 0x2B777 | sub_2B777 | 1 | 3 | int __fastcall |
| 0x2B843 | sub_2B843 | 2 | 3 | int __fastcall |
| 0x2B996 | sub_2B996 | 1 | 3 | int __fastcall |
| 0x2BB33 | sub_2BB33 | 1 | 3 | int __fastcall |
| 0x2BD6C | sub_2BD6C | 1 | 3 | int __fastcall |
| 0x2BF83 | sub_2BF83 | 1 | 3 | int __fastcall |
| 0x2BFD9 | sub_2BFD9 | 1 | 3 | int __fastcall |
| 0x2C217 | sub_2C217 | 1 | 3 | int __fastcall |
| 0x2C441 | sub_2C441 | 1 | 3 | int __fastcall |
| 0x2C67D | sub_2C67D | 1 | 3 | int __fastcall |
| 0x2CAFC | sub_2CAFC | 1 | 3 | int __fastcall |
| 0x2CCF4 | sub_2CCF4 | 1 | 3 | int __fastcall |
| 0x2CE1A | sub_2CE1A | 1 | 3 | int __fastcall |
| 0x2CF30 | sub_2CF30 | 5 | 5 | int __fastcall |
| 0x2D80D | sub_2D80D | 5 | 5 | int __fastcall |
| 0x2DF01 | sub_2DF01 | 1 | 3 | int __fastcall |
| 0x2DFC8 | sub_2DFC8 | 5 | 5 | int __fastcall |
| 0x2E2B0 | sub_2E2B0 | 5 | 5 | int __fastcall |
| 0x2E95B | sub_2E95B | 1 | 3 | int __fastcall |
| 0x2E9A8 | sub_2E9A8 | 4 | 4 | int __fastcall |
| 0x2EB9F | sub_2EB9F | 1 | 3 | int __fastcall |
| 0x2EBE1 | sub_2EBE1 | 3 | 4 | int __fastcall |
| 0x2F4D4 | sub_2F4D4 | 3 | 4 | int __fastcall |
| 0x2F631 | sub_2F631 | 3 | 4 | int __fastcall |
| 0x2F7B6 | sub_2F7B6 | 1 | 3 | int __fastcall |
| 0x2FACD | sub_2FACD | 1 | 3 | int __fastcall |
| 0x2FB2C | sub_2FB2C | 5 | 4 | int __fastcall |
| 0x2FE14 | sub_2FE14 | 3 | 4 | int __fastcall |
| 0x2FF01 | sub_2FF01 | 5 | 5 | int __fastcall |
| 0x30E25 | sub_30E25 | 1 | 3 | int __fastcall |
| 0x30E9D | sub_30E9D | 3 | 4 | int __fastcall |
| 0x311E5 | sub_311E5 | 1 | 3 | int __fastcall |
| 0x314DE | sub_314DE | 3 | 3 | int __fastcall |
| 0x31BDF | sub_31BDF | 1 | 3 | int __fastcall |
| 0x31C49 | sub_31C49 | 1 | 19 | int __fastcall |
| 0x3231B | sub_3231B | 50 | 8 | int __fastcall |
| 0x32D18 | sub_32D18 | 50 | 8 | int __fastcall |
| 0x32E8C | sub_32E8C | 50 | 8 | int __fastcall |
| 0x32FB2 | sub_32FB2 | 50 | 8 | int __fastcall |
| 0x3314B | sub_3314B | 1 | 3 | int __fastcall |
| 0x3327D | sub_3327D | 3 | 3 | int __fastcall |
| 0x3332B | sub_3332B | 2 | 2 | int __fastcall |
| 0x333F5 | sub_333F5 | 2 | 2 | int __fastcall |
| 0x3346B | sub_3346B | 1 | 2 | int __fastcall |
| 0x3347C | sub_3347C | 2 | 2 | int __fastcall |
| 0x334D9 | sub_334D9 | 4 | 3 | int __fastcall |
| 0x335A0 | sub_335A0 | 1 | 2 | int __fastcall |
| 0x335AA | sub_335AA | 2 | 2 | int __fastcall |
| 0x335DA | sub_335DA | 2 | 2 | int __fastcall |
| 0x3367E | sub_3367E | 2 | 2 | int __fastcall |
| 0x338C4 | sub_338C4 | 2 | 2 | int __fastcall |
| 0x3396A | sub_3396A | 5 | 4 | int __fastcall |
| 0x33AAE | sub_33AAE | 2 | 2 | int __fastcall |
| 0x33DBA | sub_33DBA | 2 | 2 | int __fastcall |
| 0x33E3C | sub_33E3C | 2 | 2 | int __fastcall |
| 0x33FAF | sub_33FAF | 5 | 4 | int __fastcall |
| 0x344B4 | sub_344B4 | 1 | 2 | int __fastcall |
| 0x344F2 | sub_344F2 | 1 | 2 | int __fastcall |
| 0x3460B | sub_3460B | 3 | 3 | int __fastcall |
| 0x346CD | sub_346CD | 3 | 3 | int __fastcall |
| 0x348BB | sub_348BB | 3 | 3 | int __fastcall |
| 0x348EA | sub_348EA | 3 | 2 | int __fastcall |
| 0x349EC | sub_349EC | 2 | 3 | int __fastcall |
| 0x34A1E | sub_34A1E | 3 | 2 | int __fastcall |
| 0x34A6C | sub_34A6C | 3 | 2 | int __fastcall |
| 0x34B07 | sub_34B07 | 3 | 2 | int __fastcall |
| 0x34B2F | sub_34B2F | 3 | 2 | int __fastcall |
| 0x34B6F | sub_34B6F | 2 | 2 | int __fastcall |
| 0x34B9A | sub_34B9A | 3 | 2 | int __fastcall |
| 0x34C52 | sub_34C52 | 3 | 2 | int __fastcall |
| 0x34C7A | sub_34C7A | 3 | 2 | int __fastcall |
| 0x34CF1 | sub_34CF1 | 3 | 2 | int __fastcall |
| 0x34D2F | sub_34D2F | 3 | 2 | int __fastcall |
| 0x34D64 | sub_34D64 | 3 | 2 | int __fastcall |
| 0x34D92 | sub_34D92 | 3 | 2 | int __fastcall |
| 0x34DD0 | sub_34DD0 | 3 | 2 | int __fastcall |
| 0x34EB3 | sub_34EB3 | 3 | 2 | int __fastcall |
| 0x34F38 | sub_34F38 | 3 | 2 | int __fastcall |
| 0x34F74 | sub_34F74 | 3 | 2 | int __fastcall |
| 0x34FC2 | sub_34FC2 | 3 | 2 | int __fastcall |
| 0x34FCC | sub_34FCC | 3 | 2 | int __fastcall |
| 0x350BE | sub_350BE | 3 | 2 | int __fastcall |
| 0x350C8 | sub_350C8 | 3 | 2 | int __fastcall |
| 0x351E6 | sub_351E6 | 2 | 3 | int __fastcall |
| 0x352CA | sub_352CA | 2 | 3 | int __fastcall |
| 0x353B5 | sub_353B5 | 3 | 3 | int __fastcall |
| 0x353E7 | sub_353E7 | 3 | 2 | int __fastcall |
| 0x353FA | sub_353FA | 3 | 2 | int __fastcall |
| 0x3540F | sub_3540F | 3 | 2 | int __fastcall |
| 0x3551C | sub_3551C | 3 | 2 | int __fastcall |
| 0x3553F | sub_3553F | 3 | 2 | int __fastcall |
| 0x355B7 | sub_355B7 | 3 | 2 | int __fastcall |
| 0x355F0 | sub_355F0 | 3 | 2 | int __fastcall |
| 0x356B3 | sub_356B3 | 3 | 2 | int __fastcall |
| 0x357DD | sub_357DD | 3 | 2 | int __fastcall |
| 0x359CB | sub_359CB | 2 | 3 | int __fastcall |
| 0x35A0D | sub_35A0D | 4 | 3 | int __fastcall |
| 0x35B78 | sub_35B78 | 3 | 2 | int __fastcall |
| 0x35BEE | sub_35BEE | 2 | 3 | int __fastcall |
| 0x35C1D | sub_35C1D | 2 | 3 | int __fastcall |
| 0x35C40 | sub_35C40 | 2 | 3 | int __fastcall |
| 0x35CF1 | sub_35CF1 | 2 | 3 | int __fastcall |
| 0x35D1E | sub_35D1E | 2 | 3 | int __fastcall |
| 0x35D85 | sub_35D85 | 2 | 3 | int __fastcall |
| 0x35D9E | sub_35D9E | 2 | 3 | int __fastcall |
| 0x35E0E | sub_35E0E | 2 | 3 | int __fastcall |
| 0x35E5B | sub_35E5B | 2 | 3 | int __fastcall |
| 0x35EC1 | sub_35EC1 | 2 | 3 | int __fastcall |
| 0x35F10 | sub_35F10 | 1 | 2 | int __fastcall |
| 0x35F48 | sub_35F48 | 2 | 3 | int __fastcall |
| 0x35F79 | sub_35F79 | 2 | 3 | int __fastcall |
| 0x35F88 | sub_35F88 | 2 | 3 | int __fastcall |
| 0x35FCF | sub_35FCF | 2 | 3 | int __fastcall |
| 0x360B6 | sub_360B6 | 3 | 2 | int __fastcall |
| 0x361B0 | sub_361B0 | 1 | 2 | int __fastcall |
| 0x3623C | sub_3623C | 2 | 3 | int __fastcall |
| 0x362B0 | sub_362B0 | 2 | 3 | int __fastcall |
| 0x362C5 | sub_362C5 | 2 | 3 | int __fastcall |
| 0x362E8 | sub_362E8 | 3 | 2 | int __fastcall |
| 0x363DE | sub_363DE | 1 | 2 | int __fastcall |
| 0x3642E | sub_3642E | 2 | 3 | int __fastcall |
| 0x3644E | sub_3644E | 2 | 3 | int __fastcall |
| 0x3645D | sub_3645D | 2 | 3 | int __fastcall |
| 0x364B4 | sub_364B4 | 2 | 3 | int __fastcall |
| 0x364C4 | sub_364C4 | 2 | 3 | int __fastcall |
| 0x364D4 | sub_364D4 | 2 | 3 | int __fastcall |
| 0x364FB | sub_364FB | 2 | 3 | int __fastcall |
| 0x365AB | sub_365AB | 2 | 3 | int __fastcall |
| 0x365DA | sub_365DA | 2 | 3 | int __fastcall |
| 0x3666C | sub_3666C | 1 | 2 | int __fastcall |
| 0x3669A | sub_3669A | 1 | 2 | int __fastcall |
| 0x367D1 | sub_367D1 | 1 | 2 | int __fastcall |
| 0x36C50 | sub_36C50 | 1 | 2 | int __fastcall |
| 0x36C56 | sub_36C56 | 1 | 2 | int __fastcall |
| 0x36CAB | sub_36CAB | 1 | 2 | int __fastcall |
| 0x36E3D | sub_36E3D | 1 | 2 | int __fastcall |
| 0x36E57 | sub_36E57 | 1 | 2 | int __fastcall |
| 0x36E65 | sub_36E65 | 1 | 2 | int __fastcall |
| 0x36EA7 | sub_36EA7 | 1 | 2 | int __fastcall |
| 0x36EE0 | sub_36EE0 | 1 | 2 | int __fastcall |
| 0x36F08 | sub_36F08 | 1 | 2 | int __fastcall |
| 0x36F24 | sub_36F24 | 1 | 2 | int __fastcall |
| 0x36F69 | sub_36F69 | 1 | 2 | int __fastcall |
| 0x36F82 | sub_36F82 | 1 | 2 | int __fastcall |
| 0x36FAC | sub_36FAC | 1 | 2 | int __fastcall |
| 0x36FD3 | sub_36FD3 | 1 | 2 | int __fastcall |
| 0x36FF4 | sub_36FF4 | 1 | 2 | int __fastcall |
| 0x3702F | sub_3702F | 117 | 0 | void |
| 0x3703F | sub_3703F | 1 | 0 | void |
| 0x3705F | sub_3705F | 1 | 0 | void |
| 0x373CA | sub_373CA | 18 | 0 | void |
| 0x3790A | sub_3790A | 4 | 0 | void |
| 0x37B71 | sub_37B71 | 2 | 0 | void |
| 0x37B8C | sub_37B8C | 2 | 0 | void |
| 0x37C30 | sub_37C30 | 2 | 0 | void |
| 0x37C9C | sub_37C9C | 2 | 0 | void |
| 0x37D3E | sub_37D3E | 2 | 0 | void |
| 0x37ED8 | sub_37ED8 | 2 | 0 | void |
| 0x37F70 | sub_37F70 | 2 | 0 | void |
| 0x3806A | sub_3806A | 2 | 0 | void |
| 0x3806F | sub_3806F | 2 | 0 | void |
| 0x3815F | sub_3815F | 2 | 0 | void |
| 0x381D8 | sub_381D8 | 2 | 0 | void |
| 0x382D6 | sub_382D6 | 2 | 0 | void |
| 0x382DB | sub_382DB | 2 | 0 | void |
| 0x382E9 | sub_382E9 | 2 | 0 | void |
| 0x383F1 | sub_383F1 | 2 | 0 | void |
| 0x3845E | sub_3845E | 2 | 0 | void |
| 0x387BC | sub_387BC | 2 | 0 | void |
| 0x388A7 | sub_388A7 | 2 | 0 | void |
| 0x3899A | sub_3899A | 2 | 0 | void |
| 0x38A10 | sub_38A10 | 2 | 0 | void |
| 0x38AFC | sub_38AFC | 2 | 0 | void |
| 0x38BD9 | sub_38BD9 | 2 | 0 | void |
| 0x38CA8 | sub_38CA8 | 2 | 0 | void |
| 0x38D77 | sub_38D77 | 2 | 0 | void |
| 0x38DE4 | sub_38DE4 | 2 | 0 | void |
| 0x38E46 | sub_38E46 | 2 | 0 | void |
| 0x38F2B | sub_38F2B | 2 | 0 | void |
| 0x3901E | sub_3901E | 2 | 0 | void |
| 0x3908B | sub_3908B | 2 | 0 | void |
| 0x392D0 | sub_392D0 | 2 | 0 | void |
| 0x393B5 | sub_393B5 | 2 | 0 | void |
| 0x394B4 | sub_394B4 | 2 | 0 | void |
| 0x3958E | sub_3958E | 2 | 0 | void |
| 0x398DF | sub_398DF | 2 | 0 | void |
| 0x3994C | sub_3994C | 2 | 0 | void |
| 0x399C2 | sub_399C2 | 2 | 0 | void |
| 0x39A38 | sub_39A38 | 2 | 0 | void |
| 0x39AAE | sub_39AAE | 2 | 0 | void |
| 0x39B24 | sub_39B24 | 2 | 0 | void |
| 0x39C09 | sub_39C09 | 2 | 0 | void |
| 0x39CFA | sub_39CFA | 2 | 0 | void |
| 0x39DDF | sub_39DDF | 2 | 0 | void |
| 0x39EC4 | sub_39EC4 | 2 | 0 | void |
| 0x39FA9 | sub_39FA9 | 2 | 0 | void |
| 0x3A0A2 | sub_3A0A2 | 2 | 0 | void |
| 0x3A19B | sub_3A19B | 2 | 0 | void |
| 0x3A280 | sub_3A280 | 2 | 0 | void |
| 0x3A385 | sub_3A385 | 2 | 0 | void |
| 0x3A46A | sub_3A46A | 2 | 0 | void |
| 0x3A557 | sub_3A557 | 2 | 0 | void |
| 0x3A644 | sub_3A644 | 2 | 0 | void |
| 0x3A731 | sub_3A731 | 2 | 0 | void |
| 0x3A81E | sub_3A81E | 2 | 0 | void |
| 0x3A8A0 | sub_3A8A0 | 2 | 0 | void |
| 0x3A98D | sub_3A98D | 2 | 0 | void |
| 0x3AA72 | sub_3AA72 | 2 | 0 | void |
| 0x3AB49 | sub_3AB49 | 2 | 0 | void |
| 0x3ACA3 | sub_3ACA3 | 2 | 0 | void |
| 0x3ADF5 | sub_3ADF5 | 2 | 0 | void |
| 0x3AEEE | sub_3AEEE | 2 | 0 | void |
| 0x3AF5B | sub_3AF5B | 2 | 0 | void |
| 0x3B035 | sub_3B035 | 2 | 0 | void |
| 0x3B0A2 | sub_3B0A2 | 2 | 0 | void |
| 0x3B124 | sub_3B124 | 2 | 0 | void |
| 0x3B1A6 | sub_3B1A6 | 2 | 0 | void |
| 0x3B21C | sub_3B21C | 2 | 0 | void |
| 0x3B301 | sub_3B301 | 2 | 0 | void |
| 0x3B3E6 | sub_3B3E6 | 2 | 0 | void |
| 0x3B4CB | sub_3B4CB | 2 | 0 | void |
| 0x3B5B0 | sub_3B5B0 | 2 | 0 | void |
| 0x3B716 | sub_3B716 | 2 | 0 | void |
| 0x3B80F | sub_3B80F | 2 | 0 | void |
| 0x3B908 | sub_3B908 | 2 | 0 | void |
| 0x3B98A | sub_3B98A | 2 | 0 | void |
| 0x3BA0C | sub_3BA0C | 2 | 0 | void |
| 0x3BAF1 | sub_3BAF1 | 2 | 0 | void |
| 0x3BBEA | sub_3BBEA | 2 | 0 | void |
| 0x3BCD7 | sub_3BCD7 | 2 | 0 | void |
| 0x3BDDF | sub_3BDDF | 2 | 0 | void |
| 0x3BE55 | sub_3BE55 | 2 | 0 | void |
| 0x3BF42 | sub_3BF42 | 2 | 0 | void |
| 0x3C02F | sub_3C02F | 2 | 0 | void |
| 0x3C11C | sub_3C11C | 2 | 0 | void |
| 0x3C209 | sub_3C209 | 2 | 0 | void |
| 0x3C2F6 | sub_3C2F6 | 2 | 0 | void |
| 0x3C378 | sub_3C378 | 2 | 0 | void |
| 0x3C4DB | sub_3C4DB | 2 | 0 | void |
| 0x3C5C0 | sub_3C5C0 | 2 | 0 | void |
| 0x3C636 | sub_3C636 | 2 | 0 | void |
| 0x3C6B8 | sub_3C6B8 | 2 | 0 | void |
| 0x3C7A5 | sub_3C7A5 | 2 | 0 | void |
| 0x3C8B4 | sub_3C8B4 | 2 | 0 | void |
| 0x3CA23 | sub_3CA23 | 2 | 0 | void |
| 0x3CB9B | sub_3CB9B | 2 | 0 | void |
| 0x3CBA2 | sub_3CBA2 | 2 | 0 | void |
| 0x3CF1C | sub_3CF1C | 2 | 0 | void |
| 0x3CF21 | sub_3CF21 | 2 | 0 | void |
| 0x3CF50 | sub_3CF50 | 2 | 0 | void |
| 0x3D3E5 | sub_3D3E5 | 2 | 0 | void |
| 0x3D5BE | sub_3D5BE | 2 | 0 | void |
| 0x3DB46 | sub_3DB46 | 2 | 0 | void |
| 0x3DB4C | sub_3DB4C | 2 | 0 | void |
| 0x3DE60 | sub_3DE60 | 2 | 0 | void |
| 0x3E103 | sub_3E103 | 2 | 0 | void |
| 0x3E117 | sub_3E117 | 2 | 0 | void |
| 0x3E337 | sub_3E337 | 2 | 0 | void |
| 0x3E440 | sub_3E440 | 2 | 0 | void |
| 0x3E493 | sub_3E493 | 2 | 0 | void |
| 0x3E4BC | sub_3E4BC | 2 | 0 | void |
| 0x3E4E3 | sub_3E4E3 | 2 | 0 | void |
| 0x3E536 | sub_3E536 | 2 | 0 | void |
| 0x3E602 | sub_3E602 | 2 | 0 | void |
| 0x3E619 | sub_3E619 | 2 | 0 | void |
| 0x3EA52 | sub_3EA52 | 2 | 0 | void |
| 0x3EA74 | sub_3EA74 | 2 | 0 | void |
| 0x3EA81 | sub_3EA81 | 2 | 0 | void |
| 0x3EA8E | sub_3EA8E | 2 | 0 | void |
| 0x3EBB4 | sub_3EBB4 | 2 | 0 | void |
| 0x3EBE4 | sub_3EBE4 | 2 | 0 | void |
| 0x3EC17 | sub_3EC17 | 2 | 0 | void |
| 0x3EC7C | sub_3EC7C | 2 | 0 | void |
| 0x3ED6A | sub_3ED6A | 2 | 0 | void |
| 0x3EDA5 | sub_3EDA5 | 2 | 0 | void |
| 0x3EDD1 | sub_3EDD1 | 2 | 0 | void |
| 0x3EE3D | sub_3EE3D | 2 | 0 | void |
| 0x3EE4D | sub_3EE4D | 2 | 0 | void |
| 0x3F063 | sub_3F063 | 2 | 0 | void |
| 0x3F0A8 | sub_3F0A8 | 2 | 0 | void |
| 0x3F0AC | sub_3F0AC | 2 | 0 | void |
| 0x3F113 | sub_3F113 | 2 | 0 | void |
| 0x3F1ED | sub_3F1ED | 2 | 0 | void |
| 0x3F22A | sub_3F22A | 2 | 0 | void |
| 0x3F236 | sub_3F236 | 2 | 0 | void |
| 0x3F27D | sub_3F27D | 2 | 0 | void |
| 0x3F2A5 | sub_3F2A5 | 2 | 0 | void |
| 0x3F2CC | sub_3F2CC | 2 | 0 | void |
| 0x3F2F0 | sub_3F2F0 | 2 | 0 | void |
| 0x3F320 | sub_3F320 | 2 | 0 | void |
| 0x3F344 | sub_3F344 | 2 | 0 | void |
| 0x3F374 | sub_3F374 | 2 | 0 | void |
| 0x3F398 | sub_3F398 | 2 | 0 | void |
| 0x3F3C8 | sub_3F3C8 | 2 | 0 | void |
| 0x3F3F8 | sub_3F3F8 | 2 | 0 | void |
| 0x3F43A | sub_3F43A | 2 | 0 | void |
| 0x3F446 | sub_3F446 | 2 | 0 | void |
| 0x3F4E0 | sub_3F4E0 | 2 | 0 | void |
| 0x3F565 | sub_3F565 | 2 | 0 | void |
| 0x3F5B4 | sub_3F5B4 | 2 | 0 | void |
| 0x3F656 | sub_3F656 | 2 | 0 | void |
| 0x3F937 | sub_3F937 | 2 | 0 | void |
| 0x3F950 | sub_3F950 | 2 | 0 | void |
| 0x3FB90 | sub_3FB90 | 2 | 0 | void |
| 0x3FC33 | sub_3FC33 | 2 | 0 | void |
| 0x3FCA9 | sub_3FCA9 | 2 | 0 | void |
| 0x3FDAF | sub_3FDAF | 2 | 0 | void |
| 0x3FDDC | sub_3FDDC | 2 | 0 | void |
| 0x3FEE0 | sub_3FEE0 | 2 | 0 | void |
| 0x401C0 | sub_401C0 | 2 | 0 | void |
| 0x404C0 | sub_404C0 | 2 | 0 | void |
| 0x406B0 | sub_406B0 | 2 | 0 | void |
| 0x40B40 | sub_40B40 | 2 | 0 | void |
| 0x40C40 | sub_40C40 | 2 | 0 | void |
| 0x40CF0 | sub_40CF0 | 2 | 0 | void |
| 0x413C0 | sub_413C0 | 2 | 0 | void |
| 0x414D0 | sub_414D0 | 2 | 0 | void |
| 0x414E0 | sub_414E0 | 2 | 0 | void |
| 0x415A0 | sub_415A0 | 2 | 0 | void |
| 0x415D0 | sub_415D0 | 2 | 0 | void |
| 0x416A0 | sub_416A0 | 2 | 0 | void |
| 0x416C0 | sub_416C0 | 2 | 0 | void |
| 0x416E0 | sub_416E0 | 2 | 0 | void |
| 0x416F0 | sub_416F0 | 2 | 0 | void |
| 0x417B0 | sub_417B0 | 2 | 0 | void |
| 0x418B0 | sub_418B0 | 2 | 0 | void |
| 0x419E0 | sub_419E0 | 2 | 0 | void |
| 0x41A40 | sub_41A40 | 2 | 0 | void |
| 0x41AF4 | sub_41AF4 | 2 | 0 | void |
| 0x41B1C | sub_41B1C | 2 | 0 | void |
| 0x41B26 | sub_41B26 | 2 | 0 | void |
| 0x41B84 | sub_41B84 | 2 | 0 | void |
| 0x41DDB | sub_41DDB | 2 | 0 | void |
| 0x41DEB | sub_41DEB | 2 | 0 | void |
| 0x41EC1 | sub_41EC1 | 2 | 0 | void |
| 0x41FEE | sub_41FEE | 2 | 0 | void |
| 0x420C6 | sub_420C6 | 2 | 0 | void |
| 0x420E1 | sub_420E1 | 2 | 0 | void |
| 0x422C0 | sub_422C0 | 2 | 0 | void |
| 0x423E0 | sub_423E0 | 2 | 0 | void |
| 0x424B0 | sub_424B0 | 2 | 0 | void |
| 0x427C0 | sub_427C0 | 2 | 0 | void |
| 0x42DD0 | sub_42DD0 | 2 | 0 | void |
| 0x42EA0 | sub_42EA0 | 2 | 0 | void |
| 0x42F50 | sub_42F50 | 2 | 0 | void |
| 0x439A0 | sub_439A0 | 2 | 0 | void |
| 0x43A70 | sub_43A70 | 2 | 0 | void |
| 0x43AD0 | sub_43AD0 | 2 | 0 | void |
| 0x442A0 | sub_442A0 | 2 | 0 | void |
| 0x442B0 | sub_442B0 | 2 | 0 | void |
| 0x443B0 | sub_443B0 | 2 | 0 | void |
| 0x443D0 | sub_443D0 | 2 | 0 | void |
| 0x447D0 | sub_447D0 | 2 | 0 | void |
| 0x448A0 | sub_448A0 | 2 | 0 | void |
| 0x449E0 | sub_449E0 | 2 | 0 | void |
| 0x44A60 | sub_44A60 | 2 | 0 | void |
| 0x44A70 | sub_44A70 | 2 | 0 | void |
| 0x44A90 | sub_44A90 | 2 | 0 | void |
| 0x44AA0 | sub_44AA0 | 2 | 0 | void |
| 0x44AB0 | sub_44AB0 | 2 | 0 | void |
| 0x44AF0 | sub_44AF0 | 2 | 0 | void |
| 0x44BF0 | sub_44BF0 | 2 | 0 | void |
| 0x44C30 | sub_44C30 | 2 | 0 | void |
| 0x44C70 | sub_44C70 | 2 | 0 | void |
| 0x44CA0 | sub_44CA0 | 2 | 0 | void |
| 0x44CD0 | sub_44CD0 | 2 | 0 | void |
| 0x44D10 | sub_44D10 | 2 | 0 | void |
| 0x44D80 | sub_44D80 | 2 | 0 | void |
| 0x44E10 | sub_44E10 | 2 | 0 | void |
| 0x4A3D0 | sub_4A3D0 | 0 | 0 | int |
| 0x4A3D8 | sub_4A3D8 | 0 | 0 | int |
| 0x4A3E1 | sub_4A3E1 | 0 | 0 | int |
| 0x4A3EA | sub_4A3EA | 0 | 0 | int |
| 0x4A3F3 | sub_4A3F3 | 0 | 0 | int |
| 0x4A3FC | sub_4A3FC | 0 | 0 | int |
| 0x4A407 | sub_4A407 | 0 | 0 | int |
| 0x4A412 | sub_4A412 | 0 | 0 | int |
| 0x4A41B | sub_4A41B | 0 | 0 | int |
| 0x4A424 | sub_4A424 | 0 | 0 | int |
| 0x4A45C | sub_4A45C | 0 | 0 | int |
| 0x4A461 | sub_4A461 | 0 | 0 | int |
| 0x4A480 | sub_4A480 | 0 | 0 | int |
| 0x4A49F | sub_4A49F | 0 | 0 | int |
| 0x4A4BB | sub_4A4BB | 0 | 0 | int |
| 0x4A4D7 | sub_4A4D7 | 0 | 0 | int |
| 0x4A4F6 | sub_4A4F6 | 0 | 0 | int |
| 0x4A515 | sub_4A515 | 0 | 0 | int |
| 0x4A534 | sub_4A534 | 0 | 0 | int |
| 0x4A553 | sub_4A553 | 0 | 0 | int |
| 0x4A58D | sub_4A58D | 0 | 0 | int |
| 0x4A59E | sub_4A59E | 0 | 0 | int |
| 0x4A5AF | sub_4A5AF | 0 | 0 | int |
| 0x4A5CD | sub_4A5CD | 0 | 0 | int |
| 0x4A60D | sub_4A60D | 0 | 0 | int |
| 0x4A621 | sub_4A621 | 0 | 0 | int |
| 0x4A62F | sub_4A62F | 0 | 0 | int |
| 0x4A64E | sub_4A64E | 0 | 0 | int |
| 0x4A66D | sub_4A66D | 0 | 0 | int |
| 0x4A689 | sub_4A689 | 0 | 0 | int |
| 0x4A6A5 | sub_4A6A5 | 0 | 0 | int |
| 0x4A6C4 | sub_4A6C4 | 0 | 0 | int |
| 0x4A6E3 | sub_4A6E3 | 0 | 0 | int |
| 0x4A702 | sub_4A702 | 0 | 0 | int |
| 0x4A721 | sub_4A721 | 0 | 0 | int |
| 0x4A75B | sub_4A75B | 0 | 0 | int |
| 0x4A797 | sub_4A797 | 0 | 0 | int |
| 0x4A7BB | sub_4A7BB | 0 | 0 | int |
| 0x4A7DE | sub_4A7DE | 0 | 0 | int |
| 0x4A801 | sub_4A801 | 0 | 0 | int |
| 0x4A821 | sub_4A821 | 0 | 0 | int |
| 0x4A841 | sub_4A841 | 0 | 0 | int |
| 0x4A864 | sub_4A864 | 0 | 0 | int |
| 0x4A887 | sub_4A887 | 0 | 0 | int |
| 0x4A8AA | sub_4A8AA | 0 | 0 | int |
| 0x4A8CD | sub_4A8CD | 0 | 0 | int |
| 0x4A914 | sub_4A914 | 0 | 0 | int |
| 0x4A952 | sub_4A952 | 0 | 0 | int |
| 0x4A967 | sub_4A967 | 0 | 0 | int |
| 0x4A985 | sub_4A985 | 0 | 0 | int |
| 0x4A99A | sub_4A99A | 0 | 0 | int |
| 0x4A9E1 | sub_4A9E1 | 0 | 0 | int |
| 0x4AA0C | sub_4AA0C | 0 | 0 | int |
| 0x4AA1A | sub_4AA1A | 0 | 0 | int |
| 0x4AA3B | sub_4AA3B | 0 | 0 | int |
| 0x4AA5C | sub_4AA5C | 0 | 0 | int |
| 0x4AA7A | sub_4AA7A | 0 | 0 | int |
| 0x4AA98 | sub_4AA98 | 0 | 0 | int |
| 0x4AAB9 | sub_4AAB9 | 0 | 0 | int |
| 0x4AADA | sub_4AADA | 0 | 0 | int |
| 0x4AAFB | sub_4AAFB | 0 | 0 | int |
| 0x4AB1C | sub_4AB1C | 0 | 0 | int |
| 0x4AB58 | sub_4AB58 | 0 | 0 | int |
| 0x4ABA0 | sub_4ABA0 | 0 | 0 | int |
| 0x4ABD0 | sub_4ABD0 | 0 | 0 | int |
| 0x4ABD5 | sub_4ABD5 | 0 | 0 | int |
| 0x4AC0F | sub_4AC0F | 0 | 0 | int |
| 0x4AC14 | sub_4AC14 | 0 | 0 | int |
| 0x4AC38 | sub_4AC38 | 0 | 0 | int |
| 0x4AC3D | sub_4AC3D | 0 | 0 | int |
| 0x4AC6A | sub_4AC6A | 0 | 0 | int |
| 0x4AC7F | sub_4AC7F | 0 | 0 | int |
| 0x4AC94 | sub_4AC94 | 0 | 0 | int |
| 0x4ACA6 | sub_4ACA6 | 0 | 0 | int |
| 0x4ACB8 | sub_4ACB8 | 0 | 0 | int |
| 0x4ACCD | sub_4ACCD | 0 | 0 | int |
| 0x4ACE2 | sub_4ACE2 | 0 | 0 | int |
| 0x4ACF7 | sub_4ACF7 | 0 | 0 | int |
| 0x4AD0C | sub_4AD0C | 0 | 0 | int |
| 0x4AD54 | sub_4AD54 | 0 | 0 | int |
| 0x4AD7C | sub_4AD7C | 0 | 0 | int |
| 0x4ADFD | sub_4ADFD | 0 | 0 | int |
| 0x4B018 | sub_4B018 | 0 | 0 | int |
| 0x4B122 | sub_4B122 | 0 | 0 | int |
| 0x4B23A | sub_4B23A | 0 | 0 | int |
| 0x4B2C3 | sub_4B2C3 | 0 | 0 | int |
| 0x4B31D | sub_4B31D | 0 | 0 | int |
| 0x4B332 | sub_4B332 | 0 | 0 | int |
| 0x4B347 | sub_4B347 | 0 | 0 | int |
| 0x4B35C | sub_4B35C | 0 | 0 | int |
| 0x4B371 | sub_4B371 | 0 | 0 | int |
| 0x4B386 | sub_4B386 | 0 | 0 | int |
| 0x4B39B | sub_4B39B | 0 | 0 | int |
| 0x4B3BA | sub_4B3BA | 0 | 0 | int |
| 0x4B3E6 | sub_4B3E6 | 0 | 0 | int |
| 0x4B3F8 | sub_4B3F8 | 0 | 0 | int |
| 0x4B40A | sub_4B40A | 0 | 0 | int |
| 0x4B436 | sub_4B436 | 0 | 0 | int |
| 0x4B481 | sub_4B481 | 0 | 0 | int |
| 0x4B4CC | sub_4B4CC | 0 | 0 | int |
| 0x4B555 | sub_4B555 | 0 | 0 | int |
| 0x4B5A0 | sub_4B5A0 | 0 | 0 | int |
| 0x4B5EB | sub_4B5EB | 0 | 0 | int |
| 0x4B636 | sub_4B636 | 0 | 0 | int |
| 0x4B681 | sub_4B681 | 0 | 0 | int |
| 0x4B81A | sub_4B81A | 0 | 0 | int |
| 0x4B828 | sub_4B828 | 0 | 0 | int |
| 0x4B852 | sub_4B852 | 0 | 0 | int |
| 0x4B882 | sub_4B882 | 0 | 0 | int |
| 0x4B9EE | sub_4B9EE | 0 | 0 | int |
| 0x4BA87 | sub_4BA87 | 0 | 0 | int |
| 0x4BAB1 | sub_4BAB1 | 0 | 0 | int |
| 0x4BC5C | sub_4BC5C | 0 | 0 | int |
| 0x4BC86 | sub_4BC86 | 0 | 0 | int |
| 0x4BD82 | sub_4BD82 | 0 | 0 | int |
| 0x4BD87 | sub_4BD87 | 0 | 0 | int |
| 0x4BDD7 | sub_4BDD7 | 0 | 0 | int |
| 0x4BDDC | sub_4BDDC | 0 | 0 | int |
| 0x4BE46 | sub_4BE46 | 0 | 0 | int |
| 0x4BE57 | sub_4BE57 | 0 | 0 | int |
| 0x4BE8C | sub_4BE8C | 0 | 0 | int |
| 0x4BF0C | sub_4BF0C | 0 | 0 | int |
| 0x4BF8B | sub_4BF8B | 0 | 0 | int |
| 0x4C052 | sub_4C052 | 0 | 0 | int |
| 0x4C117 | sub_4C117 | 0 | 0 | int |
| 0x4C2A4 | sub_4C2A4 | 0 | 0 | int |
| 0x4C35A | sub_4C35A | 0 | 0 | int |
| 0x4C4BD | sub_4C4BD | 0 | 0 | int |
| 0x4C523 | sub_4C523 | 0 | 0 | int |
| 0x4C59E | sub_4C59E | 0 | 0 | int |
| 0x4C68C | sub_4C68C | 0 | 0 | int |
| 0x4C6A5 | sub_4C6A5 | 0 | 0 | int |
| 0x4C6E8 | sub_4C6E8 | 0 | 0 | int |
| 0x4C980 | sub_4C980 | 0 | 0 | int |
| 0x4CB77 | sub_4CB77 | 0 | 0 | int |
| 0x4CB81 | sub_4CB81 | 0 | 0 | int |
| 0x4CB8E | sub_4CB8E | 0 | 0 | int |
| 0x4CBC4 | sub_4CBC4 | 0 | 0 | int |
| 0x4CD98 | sub_4CD98 | 0 | 0 | int |
| 0x4CE64 | sub_4CE64 | 0 | 0 | int |
| 0x4CE84 | sub_4CE84 | 0 | 0 | int |
| 0x4CED6 | sub_4CED6 | 0 | 0 | int |
| 0x4CF1E | sub_4CF1E | 0 | 0 | int |
| 0x4CF4D | sub_4CF4D | 0 | 0 | int |
| 0x4D690 | sub_4D690 | 0 | 0 | int |
| 0x4D966 | sub_4D966 | 0 | 0 | int |
| 0x4DED4 | sub_4DED4 | 0 | 0 | int |
| 0x4DEEC | sub_4DEEC | 0 | 0 | int |
| 0x4DF09 | sub_4DF09 | 0 | 0 | int |
| 0x4DF28 | sub_4DF28 | 0 | 0 | int |
| 0x4DF4C | sub_4DF4C | 2 | 0 | int |
| 0x4DF84 | sub_4DF84 | 0 | 0 | int |
| 0x4E016 | sub_4E016 | 0 | 0 | int |
| 0x4E0A2 | sub_4E0A2 | 0 | 0 | int |
| 0x4E127 | sub_4E127 | 0 | 0 | int |
| 0x4E1A6 | sub_4E1A6 | 2 | 0 | int |
| 0x4E22A | sub_4E22A | 2 | 0 | int |
| 0x4E29C | sub_4E29C | 0 | 0 | int |
| 0x4E310 | sub_4E310 | 0 | 0 | int |
| 0x4E31C | sub_4E31C | 6 | 0 | int |
| 0x4E381 | sub_4E381 | 4 | 0 | int |
| 0x4E390 | sub_4E390 | 0 | 0 | int |
| 0x4E42C | sub_4E42C | 0 | 0 | int |
| 0x4E4BE | sub_4E4BE | 0 | 0 | int |
| 0x4E4F6 | sub_4E4F6 | 0 | 0 | int |
| 0x4E5CC | sub_4E5CC | 0 | 0 | int |
| 0x4E680 | sub_4E680 | 0 | 0 | int |
| 0x4E703 | sub_4E703 | 0 | 0 | int |
| 0x4E71F | sub_4E71F | 0 | 0 | int |
| 0x4E751 | sub_4E751 | 0 | 0 | int |
| 0x4E795 | sub_4E795 | 0 | 0 | int |
| 0x4E7DD | sub_4E7DD | 0 | 0 | int |
| 0x4E7F2 | sub_4E7F2 | 0 | 0 | int |
| 0x4E809 | sub_4E809 | 0 | 0 | int |
| 0x4E821 | sub_4E821 | 4 | 0 | int |
| 0x4E838 | sub_4E838 | 4 | 0 | int |
| 0x4E84F | sub_4E84F | 2 | 0 | int |
| 0x4E866 | sub_4E866 | 2 | 0 | int |
| 0x4E87D | sub_4E87D | 0 | 0 | int |
| 0x4E88E | sub_4E88E | 0 | 0 | int |
| 0x4E8A5 | sub_4E8A5 | 3 | 0 | int |
| 0x4E8BC | sub_4E8BC | 4 | 0 | int |
| 0x4E8D3 | sub_4E8D3 | 0 | 0 | int |
| 0x4E98D | sub_4E98D | 0 | 0 | int |
| 0x4EB48 | sub_4EB48 | 2 | 0 | int |
| 0x4EB59 | sub_4EB59 | 0 | 0 | int |
| 0x4EBAB | sub_4EBAB | 0 | 0 | int |
| 0x4EBE3 | sub_4EBE3 | 0 | 0 | int |
| 0x4EBFF | sub_4EBFF | 0 | 0 | int |
| 0x4EC31 | sub_4EC31 | 0 | 0 | int |
| 0x4EC65 | sub_4EC65 | 0 | 0 | int |
| 0x4EC66 | sub_4EC66 | 0 | 0 | int |
| 0x4EC7C | sub_4EC7C | 0 | 0 | int |
| 0x4ECA4 | sub_4ECA4 | 0 | 0 | int |
| 0x4ECBF | sub_4ECBF | 0 | 0 | int |
| 0x4ECF0 | sub_4ECF0 | 0 | 0 | int |
| 0x4ED0B | sub_4ED0B | 0 | 0 | int |
| 0x4ED34 | sub_4ED34 | 0 | 0 | int |
| 0x4ED4F | sub_4ED4F | 0 | 0 | int |
| 0x4ED7A | sub_4ED7A | 0 | 0 | int |
| 0x4EEE0 | sub_4EEE0 | 0 | 0 | int |

---

## 函数分类

### 按调用次数排序（被调用最多的函数）

| 函数 | 地址 | 被调用次数 |
|------|------|-----------|
| sub_117E7 | 0x117E7 | 106 |
| sub_11CAC | 0x11CAC | 50 |
| sub_3702F | 0x3702F | 117 |
| sub_135DD | 0x135DD | 51 |
| sub_1366A | 0x1366A | 49 |
| sub_235BC | 0x235BC | 49 |
| sub_235F9 | 0x235F9 | 49 |
| sub_237D5 | 0x237D5 | 49 |
| sub_238DC | 0x238DC | 49 |
| sub_23A0A | 0x23A0A | 49 |
| sub_23B5F | 0x23B5F | 49 |
| sub_23CD5 | 0x23CD5 | 49 |
| sub_23E74 | 0x23E74 | 49 |
| sub_240FA | 0x240FA | 49 |
| sub_24DF2 | 0x24DF2 | 49 |
| sub_3231B | 0x3231B | 49 |
| sub_32D18 | 0x32D18 | 49 |
| sub_32E8C | 0x32E8C | 49 |
| sub_32FB2 | 0x32FB2 | 49 |

### 按调用者数量排序（调用其他函数最多的函数）

| 函数 | 地址 | 调用者数 |
|------|------|----------|
| sub_117E7 | 0x117E7 | 1 |
| sub_10B4E | 0x10B4E | 46 |

---

## 地址范围分类

| 地址范围 | 推测功能区 |
|----------|-----------|
| 0x10000 - 0x107FF | 程序入口和初始化 |
| 0x10800 - 0x10FFF | 游戏数据加载 |
| 0x11000 - 0x11FFF | 输入处理和主循环 |
| 0x12000 - 0x13FFF | 渲染和绘图 |
| 0x14000 - 0x17FFF | 游戏逻辑和战斗 |
| 0x18000 - 0x1FFFF | 单元处理和动画 |
| 0x20000 - 0x23FFF | 游戏主循环函数群 |
| 0x24000 - 0x2FFFF | 扩展游戏逻辑 |
| 0x30000 - 0x33FFF | UI和菜单函数 |
| 0x34000 - 0x36FFF | 辅助函数 |
| 0x37000 - 0x3FFFF | 系统级函数（内存管理等） |
| 0x40000 - 0x44FFF | 数据表和常量 |
| 0x4A000 - 0x4EFFF | 工具函数和辅助 |

---

*文档生成时间: 2026-04-07*
*基于 IDA Pro 反汇编分析和 decompile 工具输出*
*共 1228 个函数*