用到的三方的库：[libraw](https://www.libraw.org/)

查询 Tiff tag ID 含义的网站： [TIFF Tag Reference](https://www.awaresystems.be/imaging/tiff/tifftags.html)

## Tiff TAG ID 中常用的 TAG

一些常见的TIFF标签及其含义包括但不限于：
- 256 (ImageWidth): 图像的宽度，以像素为单位。
- 257 (ImageLength): 图像的高度，以像素为单位。
- 258 (BitsPerSample): 每个样本的位数，对于灰度图像通常是8或16，对于RGB图像，每个通道也是8或16位。
- 259 (Compression): 图像数据的压缩方案，如无压缩(1)、CCITT Group 3传真(2)、CCITT Group 4传真(4)、LZW(5)、JPEG(6)等。
- 270 (ImageDescription): 图像描述文本。
- 271 (Make) 和 272 (Model): 拍摄设备制造商和型号。
- 273 (StripOffsets): 数据条带（strip）在文件中的偏移量，用于定位实际图像数据。
- 277 (SamplesPerPixel): 每像素的样本数，对于RGB图像通常为3。

## 一些 TAG ID 的详细介绍

## 259: 压缩类型
在TIFF格式中，Tag ID 259 对应于 `Compression` 标签，它定义了图像数据的压缩方案。这个标签是一个SHORT类型，长度为1，其可取值及其含义如下：

1. **1 (No compression)**: 表示图像数据未经过压缩。
2. **2 (CCITT 1D)**: 使用CCITT Group 3 1-Dimensional Modified Huffman Run-Length Encoding压缩，常见于传真图像。
3. **3 (CCITT Group 3 Fax)**: 使用CCITT Group 3传真编码，这是一种较老的压缩标准。
4. **4 (CCITT Group 4 Fax)**: 使用CCITT Group 4传真编码，提供更好的压缩效率，适用于二值图像。
5. **5 (LZW)**: 使用Lempel-Ziv-Welch压缩算法，适用于颜色或灰度图像，提供较好的压缩率。
6. **6 (Old JPEG)**: 使用旧版的JPEG压缩，不常用，已被JPEG压缩（值为7）替代。
7. **7 (JPEG DCT)**: 使用JPEG Discrete Cosine Transform压缩，广泛应用于彩色或灰度连续色调图像。
8. **8 (Adobe Deflate)**: 使用Deflate压缩算法，类似于ZIP压缩。
9. **32773 (PackBits)**: 一种简单的Run-Length Encoding压缩，特别适合于二值图像和具有大量相同值区域的图像。
10. **32809 (Thunderscan)**: 一种较老的压缩格式，主要用于某些扫描仪产生的图像。
11. **32946 (Kodak DCS)**: 专为Kodak Digital Camera System设计的压缩方法。
12. **34665 (JPEG-LS)**: 使用JPEG-LS（Lossless JPEG）压缩，提供无损或近似无损压缩。
13. **34712 (JPEG 2000)**: 使用JPEG 2000压缩标准，支持高压缩率和无损压缩。
14. **34892 (JPEG XR)**: 使用JPEG XR（以前称为Windows Media Photo）压缩，提供宽动态范围和高色深支持。
15. **50000-50999**（及更大值）: 这个范围及以上的值保留给私有或特殊用途的压缩方法。

每种压缩类型都有其适用场景和特点，选择合适的压缩方式可以根据图像内容和使用需求平衡图像质量和文件大小。
