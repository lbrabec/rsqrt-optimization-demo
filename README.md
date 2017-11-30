# rsqrt-optimization-demo
Inverse square root optimization demo

```
+------------+--------+--------+
|     Method | Result |   Time |
+------------+--------+--------+
| Naive math | 5793.1 | 42.1ms |
|    Carmack | 5779.0 | 55.9ms |
| SSE scalar | 5793.0 | 25.5ms |
| SSE packed | 5785.3 |  7.6ms |
| AVX packed | 5796.8 |  4.5ms |
+------------+--------+--------+
```