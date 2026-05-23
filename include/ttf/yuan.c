#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

//这是AI编写的小工具, 如果您需要更改默认字体的话
// BMPFontLoader.c
// BMP图片字体数据转换工具 - 控制台版本
// 拖拽BMP文件到程序上，自动生成同名的txt文件
// 图片要求: 8列 x 16行 字符 (128个字符), 每个字符16x16像素，总尺寸 128x256 像素
// 输出纹理布局: 128字符横向排列，但按行交错存储 (2048 x 16)
// 每个字符的第0行连续存储，然后是所有字符的第1行，以此类推

#include <stdio.h>
#include <windows.h>

#define CHAR_SIZE 16        // 每个字符16x16像素
#define CHAR_TOTAL 128      // ASCII 0-127 共128个字符

// 字体数据: 重新排列后的格式 [行][字符][列] 或者简化为一维数组
// 最终纹理: 2048 x 16 像素，每行 2048 字节（RGBA）
static unsigned char texture_data[CHAR_SIZE][CHAR_TOTAL * CHAR_SIZE * 4]; // [16行][2048像素*4字节]
static unsigned char font_buffer[CHAR_TOTAL][CHAR_SIZE][CHAR_SIZE][4];    // 临时存储 [字符][行][列][RGBA]

// 从BMP图片读取字体数据
int LoadFromBMP(const char* filename)
{
    wchar_t wfilename[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, MAX_PATH);

    HBITMAP hBitmap = (HBITMAP)LoadImageW(NULL, wfilename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!hBitmap)
    {
        printf("错误: 无法加载图片 [%s]\n", filename);
        return 0;
    }

    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    int expected_width = 8 * CHAR_SIZE;
    int expected_height = 16 * CHAR_SIZE;

    if (bitmap.bmWidth != expected_width || bitmap.bmHeight != expected_height)
    {
        printf("错误: 图片尺寸不正确! 期望: %dx%d, 实际: %dx%d\n",
            expected_width, expected_height, bitmap.bmWidth, bitmap.bmHeight);
        DeleteObject(hBitmap);
        return 0;
    }

    HDC hdc = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hBitmap);

    printf("正在读取字体数据...\n");

    // 读取每个字符的原始像素
    for (int c = 0; c < CHAR_TOTAL; c++)
    {
        int col = c % 8;
        int row = c / 8;
        int start_x = col * CHAR_SIZE;
        int start_y = row * CHAR_SIZE;

        for (int y = 0; y < CHAR_SIZE; y++)
        {
            for (int x = 0; x < CHAR_SIZE; x++)
            {
                COLORREF pixel = GetPixel(hdcMem, start_x + x, start_y + y);
                int r = GetRValue(pixel);
                int g = GetGValue(pixel);
                int b = GetBValue(pixel);

                if (r == 0 && g == 0 && b == 0)
                {
                    font_buffer[c][y][x][0] = 0;
                    font_buffer[c][y][x][1] = 0;
                    font_buffer[c][y][x][2] = 0;
                    font_buffer[c][y][x][3] = 0;
                }
                else
                {
                    font_buffer[c][y][x][0] = r;
                    font_buffer[c][y][x][1] = g;
                    font_buffer[c][y][x][2] = b;
                    font_buffer[c][y][x][3] = 255;
                }
            }
        }
    }

    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdc);
    DeleteObject(hBitmap);

    printf("重新排列纹理数据 (按行交错存储)...\n");

    // 重新排列：所有字符的第0行连续存储，然后是所有字符的第1行...
    // texture_data[行][字符索引 * 16 + 像素列]
    for (int y = 0; y < CHAR_SIZE; y++)           // 16行
    {
        for (int c = 0; c < CHAR_TOTAL; c++)      // 128个字符
        {
            for (int x = 0; x < CHAR_SIZE; x++)   // 16列
            {
                int pixel_index = c * CHAR_SIZE + x;
                texture_data[y][pixel_index * 4 + 0] = font_buffer[c][y][x][0]; // R
                texture_data[y][pixel_index * 4 + 1] = font_buffer[c][y][x][1]; // G
                texture_data[y][pixel_index * 4 + 2] = font_buffer[c][y][x][2]; // B
                texture_data[y][pixel_index * 4 + 3] = font_buffer[c][y][x][3]; // A
            }
        }
    }

    printf("完成!\n");
    return 1;
}

// 保存所有字符数据到文件
void SaveToFile(const char* input_filename)
{
    char output_filename[MAX_PATH];
    strcpy(output_filename, input_filename);
    char* dot = strrchr(output_filename, '.');
    if (dot) *dot = '\0';
    strcat(output_filename, ".txt");

    FILE* fp = fopen(output_filename, "w");
    if (!fp)
    {
        printf("错误: 无法创建文件 [%s]\n", output_filename);
        return;
    }

    printf("正在保存到 [%s]...\n", output_filename);

    fprintf(fp, "// 字体纹理数据 - 128个字符(ASCII 0-127), 每个字符16x16像素, RGBA格式\n");
    fprintf(fp, "// 纹理尺寸: 2048 x 16 像素\n");
    fprintf(fp, "// 布局方式: 按行交错存储\n");
    fprintf(fp, "//   - 第0行: 所有字符的第0行像素 (字符0,1,2...127)\n");
    fprintf(fp, "//   - 第1行: 所有字符的第1行像素\n");
    fprintf(fp, "//   - ...\n");
    fprintf(fp, "//   - 第15行: 所有字符的第15行像素\n");
    fprintf(fp, "// 像素位置: x = ascii * 16 + offset_x, y = offset_y\n");
    fprintf(fp, "// 黑色(0,0,0) -> 透明(0x00,0x00,0x00,0x00)\n");
    fprintf(fp, "// 其他颜色 -> 保持原始RGB, Alpha=255\n\n");
    fprintf(fp, "// 总大小: 2048 * 16 * 4 = 131072 字节\n\n");
    fprintf(fp, "unsigned char moon_simple_font_data[131072] = \n{\n");

    for (int y = 0; y < CHAR_SIZE; y++)  // 16行
    {
        fprintf(fp, "    // 第 %d 行 - 所有字符的第 %d 行\n", y, y);

        for (int pixel = 0; pixel < CHAR_TOTAL * CHAR_SIZE; pixel++)  // 2048个像素
        {
            if (pixel % 16 == 0 && pixel > 0)
                fprintf(fp, "    // 字符 %d\n", (pixel / 16) - 1);

            fprintf(fp, "    0x%02X,0x%02X,0x%02X,0x%02X",
                texture_data[y][pixel * 4 + 0],
                texture_data[y][pixel * 4 + 1],
                texture_data[y][pixel * 4 + 2],
                texture_data[y][pixel * 4 + 3]);

            if (!(y == CHAR_SIZE - 1 && pixel == CHAR_TOTAL * CHAR_SIZE - 1))
                fprintf(fp, ",");

            fprintf(fp, "\n");
        }

        if (y < CHAR_SIZE - 1)
            fprintf(fp, "\n");
    }

    fprintf(fp, "};\n");
    fclose(fp);

    printf("保存完成! 文件大小: 131072 字节\n");
}

void ShowUsage(void)
{
    printf("========================================\n");
    printf("   BMP 字体数据转换工具\n");
    printf("========================================\n");
    printf("使用方法:\n");
    printf("  将 BMP 图片拖拽到此程序上\n");
    printf("\n");
    printf("图片要求:\n");
    printf("  - 尺寸: 128x256 像素\n");
    printf("  - 布局: 8列 x 16行字符\n");
    printf("  - 每字符: 16x16 像素\n");
    printf("  - 共128个字符 (ASCII 0-127)\n");
    printf("\n");
    printf("输出纹理:\n");
    printf("  - 尺寸: 2048 x 16 像素\n");
    printf("  - 布局: 按行交错存储\n");
    printf("  - 采样: tex2D(u_texture, vec2(asii/128.0, row/16.0))\n");
    printf("========================================\n");
}

int main(int argc, char* argv[])
{
    printf("BMP Font Loader - 字体数据转换工具\n\n");

    if (argc < 2)
    {
        ShowUsage();
        printf("\n按任意键退出...");
        getchar();
        return 0;
    }

    for (int i = 1; i < argc; i++)
    {
        printf("\n----------------------------------------\n");
        printf("处理文件 [%d/%d]: %s\n", i, argc - 1, argv[i]);
        printf("----------------------------------------\n");

        const char* ext = strrchr(argv[i], '.');
        if (!ext || (stricmp(ext, ".bmp") != 0))
        {
            printf("警告: 跳过非BMP文件 [%s]\n", argv[i]);
            continue;
        }

        if (LoadFromBMP(argv[i]))
        {
            SaveToFile(argv[i]);
            printf("成功!\n");
        }
        else
        {
            printf("失败! 跳过此文件.\n");
        }
    }

    printf("\n========================================\n");
    printf("所有处理完成!\n");
    printf("========================================\n");
    printf("\n按任意键退出...");
    getchar();
    return 0;
}