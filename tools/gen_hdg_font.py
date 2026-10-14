# -*- coding: utf-8 -*-
"""为 HDG12864F1(SED1565,128x64)生成 16x16 汉字点阵，输出可直接贴进 HDG12864F1.c。

字模格式（反解自本工程已有的"温"字并验证过）
------------------------------------------------
一个汉字 = Upper[16] + Lower[16]，共 32 字节：

    Upper = 上半页（第 0..7 行）
    Lower = 下半页（第 8..15 行）
    每个字节 = **一列**的 8 个像素，数组下标 0..15 就是第 0..15 列
    bit0 = 该页最上面那一行，bit7 = 最下面那一行

对应的写屏调用（见 HDG12864F1_WriteChineseCharUpper/Lower）：

    HDG12864F1_WriteChineseCharUpper(cCharXianUpper, CHINESE_COLUMN_n, page);
    HDG12864F1_WriteChineseCharLower(cCharXianLower, CHINESE_COLUMN_n, page);

用法
----
    python tools/gen_hdg_font.py                # 生成 CHARS 里的字
    python tools/gen_hdg_font.py 显示模式        # 只生成指定的字

依赖 Pillow 和 C:\\Windows\\Fonts 下的中文字体。
"""
import os
import sys

from PIL import Image, ImageDraw, ImageFont

# 界面用到的字形
CHARS = "显示模式设置"

# 变量名映射。注意重名：湿 已占用 cCharShi，值 已占用 cCharZhi，
# 所以 示/式/置 用数字后缀区分。
NAMES = {
    "显": "cCharXian",
    "示": "cCharShi2",
    "模": "cCharMo",
    "式": "cCharShi3",
    "设": "cCharShe",
    "置": "cCharZhi2",
}

FONT_CANDIDATES = [
    r"C:\Windows\Fonts\simsun.ttc",
]


def find_font():
    for p in FONT_CANDIDATES:
        if os.path.exists(p):
            return p
    raise SystemExit("未找到中文字体文件")


def render_16x16(ch, font):
    """渲染成 16x16 的 0/1 矩阵，返回 16 个 uint16（bit15 = 最左边一列）。

    必须「宋体 16px 直接渲染、不缩放」—— 本工程原有的字模就是这么来的，
    实测渲染「温」字与 HDG12864F1.c 里的数据一个像素都不差。
    换成黑体、或者加 bbox 裁剪缩放，复杂字（模/置）会糊成一团。
    """
    img = Image.new("L", (16, 16), 0)
    ImageDraw.Draw(img).text((0, 0), ch, font=font, fill=255)

    px = img.load()
    rows = []
    for y in range(16):
        row = 0
        for x in range(16):
            if px[x, y] > 128:
                row |= 1 << (15 - x)
        rows.append(row)
    return rows


def pack(rows):
    """16x16 行矩阵 -> (Upper[16], Lower[16])，列优先、bit0 在页顶。"""
    upper, lower = [], []
    for c in range(16):
        xbit = 15 - c                       # 该列在每一个 row 里的位
        bu = bl = 0
        for r in range(8):
            if (rows[r] >> xbit) & 1:
                bu |= 1 << r
        for r in range(8, 16):
            if (rows[r] >> xbit) & 1:
                bl |= 1 << (r - 8)
        upper.append(bu)
        lower.append(bl)
    return upper, lower


def art(rows):
    """行矩阵打印成 ASCII 图，方便肉眼确认。"""
    return "\n".join("  " + "".join("#" if (r >> (15 - x)) & 1 else "." for x in range(16))
                     for r in rows)


def fmt_array(name, data):
    body = ",".join("0x%02X" % b for b in data)
    return "uint8_t %s[16] = {\n%s\n};" % (name, body)


def main():
    chars = sys.argv[1] if len(sys.argv) > 1 else CHARS
    font_path = find_font()
    font = ImageFont.truetype(font_path, 16)

    unknown = [c for c in chars if c not in NAMES]
    if unknown:
        raise SystemExit("这些字没在 NAMES 里登记，先补一下: %s" % "".join(unknown))

    out = []
    for ch in chars:
        rows = render_16x16(ch, font)
        if rows is None:
            print("跳过（渲染为空）:", ch)
            continue
        up, lo = pack(rows)
        name = NAMES[ch]
        print("/* %s  %s */" % (name, ch))
        print(art(rows))
        print(fmt_array(name + "Upper", up) + "   // 「%s」的上半部分" % ch)
        print(fmt_array(name + "Lower", lo) + "   // 「%s」的下半部分" % ch)
        print()
        out.append((name, up, lo))

    print("// ==== 共 %d 个字，%d 字节 ====" % (len(out), len(out) * 32))
    print("// HDG12864F1.h 里要加 %d 条 extern:" % (len(out) * 2))
    for name, _, _ in out:
        print("//   extern uint8_t %sUpper[16];  extern uint8_t %sLower[16];" % (name, name))


if __name__ == "__main__":
    main()
