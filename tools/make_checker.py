from PIL import Image

size, cell = 64, 32          # 画像は64x64、1マスは32px → 2x2マスの模様
img = Image.new("RGB", (size, size))
for y in range(size):
    for x in range(size):
        light = ((x // cell) + (y // cell)) % 2 == 0
        img.putpixel((x, y), (200, 200, 200) if light else (90, 90, 90))
img.save("Assets/Textures/checker.bmp")