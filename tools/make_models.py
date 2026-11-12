# make_models.py
# ペンギンとシロクマのOBJモデル、色見本テクスチャ(palette.bmp)を生成する。
#
# 使い方(ActionEngineのフォルダで):
#     python3 tools/make_models.py
#
# 出力:
#     Assets/Models/penguin_blue.obj
#     Assets/Models/polar_bear.obj
#     Assets/Textures/palette.bmp
#
# 約束ごと:
#   - 座標系はエンジンと同じ(x:右, y:上, z:前)。モデルは +z を向き、足元が y = 0
#   - 面の向き:(b - a) × (c - a) が外側を向く並び(Culling.cpp の判定と同じ式)。
#     パーツごとに符号付き体積を計算し、負なら全部の面を裏返すので、
#     並び順(ワインディング)の間違いは起きない
#   - 色:三角形ごとに1色。palette.bmp の縦じまの1本を UV で指す
#   - 標準ライブラリだけで動く(PILやnumpyは不要)

import math
import os
import struct

# ---------------------------------------------------------------------------
# 色見本(パレット)
# ---------------------------------------------------------------------------
PALETTE = [
    ("penguin_back",  (45, 85, 170)),    # 0 ペンギンの背中(青)
    ("penguin_belly", (246, 246, 240)),  # 1 ペンギンのお腹(白)
    ("orange",        (245, 150, 40)),   # 2 くちばし・足
    ("black",         (28, 28, 34)),     # 3 目・鼻
    ("yellow",        (250, 212, 90)),   # 4 ペンギンの胸の黄色
    ("bear_fur",      (240, 240, 232)),  # 5 シロクマの毛
    ("bear_shade",    (214, 211, 198)),  # 6 シロクマの鼻先・足先
    ("dark_grey",     (88, 84, 84)),     # 7 耳の内側・爪
    ("eye_white",     (255, 255, 255)),  # 8 白目
    ("penguin_dark",  (30, 55, 120)),    # 9 ペンギンの頭・翼の先(濃い青)
    ("orange_dark",   (215, 110, 30)),   # 10 くちばしの下側
]
PALETTE_SLOTS = 16   # 縦じまの本数(余りは予備)
PALETTE_SIZE = 128   # 画像の一辺(px)
COLOR = {name: i for i, (name, _) in enumerate(PALETTE)}


def palette_uv(index):
    """色番号 → その縦じまの真ん中を指すUV"""
    return ((index + 0.5) / PALETTE_SLOTS, 0.5)


def write_palette_bmp(path):
    """24bit無圧縮BMPを書き出す(BMPloader.cpp が読める形式)"""
    size = PALETTE_SIZE
    stripe = size // PALETTE_SLOTS
    row = bytearray()
    for x in range(size):
        i = x // stripe
        r, g, b = PALETTE[i][1] if i < len(PALETTE) else (255, 0, 255)
        row += bytes((b, g, r))  # BMPはBGRの順
    row += bytes((4 - len(row) % 4) % 4)  # 1行は4バイト境界にそろえる
    pixels = bytes(row) * size
    header = struct.pack("<2sIHHI", b"BM", 54 + len(pixels), 0, 0, 54)
    info = struct.pack("<IiiHHIIiiII", 40, size, size, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    with open(path, "wb") as f:
        f.write(header + info + pixels)


# ---------------------------------------------------------------------------
# ベクトルの小道具
# ---------------------------------------------------------------------------
def add(a, b): return (a[0] + b[0], a[1] + b[1], a[2] + b[2])
def sub(a, b): return (a[0] - b[0], a[1] - b[1], a[2] - b[2])
def mul(a, s): return (a[0] * s, a[1] * s, a[2] * s)
def dot(a, b): return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def normalize(a):
    l = math.sqrt(dot(a, a))
    return (0.0, 1.0, 0.0) if l < 1e-12 else (a[0] / l, a[1] / l, a[2] / l)


def rot_x(p, deg):
    c, s = math.cos(math.radians(deg)), math.sin(math.radians(deg))
    return (p[0], p[1] * c - p[2] * s, p[1] * s + p[2] * c)


def rot_y(p, deg):
    c, s = math.cos(math.radians(deg)), math.sin(math.radians(deg))
    return (p[0] * c + p[2] * s, p[1], -p[0] * s + p[2] * c)


def rot_z(p, deg):
    c, s = math.cos(math.radians(deg)), math.sin(math.radians(deg))
    return (p[0] * c - p[1] * s, p[0] * s + p[1] * c, p[2])


# ---------------------------------------------------------------------------
# パーツ(閉じた立体1個)とモデル
# ---------------------------------------------------------------------------
class Part:
    def __init__(self):
        self.verts = []   # (x, y, z)
        self.tris = []    # (ia, ib, ic)
        self.colors = []  # 三角形ごとの色番号
        self.quad_tris = 0  # 先頭からこの枚数までは、2枚で1つの四角形になっている

    def transform(self, fn):
        self.verts = [fn(v) for v in self.verts]
        return self

    def move(self, offset):
        return self.transform(lambda v: add(v, offset))

    def signed_volume(self):
        vol = 0.0
        for a, b, c in self.tris:
            vol += dot(self.verts[a], cross(self.verts[b], self.verts[c]))
        return vol / 6.0

    def fix_winding(self):
        """(b-a)×(c-a) が外を向くようにそろえる。体積が負なら全部裏返す"""
        if self.signed_volume() < 0.0:
            self.tris = [(a, c, b) for a, b, c in self.tris]

    def paint(self, fn):
        """fn(重心, 外向き法線) → 色番号 で、三角形ごとに色を塗る"""
        self.fix_winding()

        def info(i):
            a, b, c = self.tris[i]
            pa, pb, pc = self.verts[a], self.verts[b], self.verts[c]
            return mul(add(add(pa, pb), pc), 1.0 / 3.0), cross(sub(pb, pa), sub(pc, pa))

        i = 0
        while i < len(self.tris):
            if i < self.quad_tris:
                # 四角形は2枚まとめて1色にする(色の境目がギザギザにならないように)
                (c0, n0), (c1, n1) = info(i), info(i + 1)
                color = fn(mul(add(c0, c1), 0.5), normalize(add(n0, n1)))
                self.colors[i] = self.colors[i + 1] = color
                i += 2
            else:
                centroid, normal = info(i)
                self.colors[i] = fn(centroid, normalize(normal))
                i += 1
        return self


def catmull_rom(p0, p1, p2, p3, t):
    """4つの数の組のあいだを、なめらかに補間する"""
    t2, t3 = t * t, t * t * t
    return tuple(
        0.5 * ((2 * b) + (-a + c) * t + (2 * a - 5 * b + 4 * c - d) * t2 + (-a + 3 * b - 3 * c + d) * t3)
        for a, b, c, d in zip(p0, p1, p2, p3)
    )


def smooth_sections(sections, subdiv):
    """断面のリストを、スプラインで細かくする"""
    if subdiv <= 1:
        return list(sections)
    out = []
    n = len(sections)
    for i in range(n - 1):
        p0 = sections[max(i - 1, 0)]
        p1, p2 = sections[i], sections[i + 1]
        p3 = sections[min(i + 2, n - 1)]
        for k in range(subdiv):
            out.append(catmull_rom(p0, p1, p2, p3, k / subdiv))
    out.append(sections[-1])
    return out


def loft(sections, segments, color, axis="y", subdiv=2):
    """だ円の輪を積み重ねて、両端をふさいだ立体を作る。

    sections: (t, ru, rv, cu, cv) のリスト
        t  … 軸に沿った位置
        ru, rv … 輪の半径(2方向)
        cu, cv … 輪の中心のずれ
    axis = "y": 輪はxz平面(u = x, v = z)、tはy
    axis = "z": 輪はxy平面(u = x, v = y)、tはz
    """
    secs = smooth_sections(sections, subdiv)
    part = Part()

    def place(t, u, v):
        return (u, t, v) if axis == "y" else (u, v, t)

    for (t, ru, rv, cu, cv) in secs:
        ru, rv = max(ru, 1e-4), max(rv, 1e-4)
        for s in range(segments):
            ang = 2.0 * math.pi * s / segments
            part.verts.append(place(t, cu + ru * math.cos(ang), cv + rv * math.sin(ang)))

    rings = len(secs)
    for r in range(rings - 1):
        for s in range(segments):
            a = r * segments + s
            b = r * segments + (s + 1) % segments
            c = (r + 1) * segments + (s + 1) % segments
            d = (r + 1) * segments + s
            part.tris += [(a, b, c), (a, c, d)]

    part.quad_tris = len(part.tris)

    # 両端のフタ(中心の点から放射状に)
    first, last = secs[0], secs[-1]
    i_first = len(part.verts)
    part.verts.append(place(first[0], first[3], first[4]))
    i_last = len(part.verts)
    part.verts.append(place(last[0], last[3], last[4]))
    base = (rings - 1) * segments
    for s in range(segments):
        n = (s + 1) % segments
        part.tris.append((i_first, n, s))
        part.tris.append((i_last, base + s, base + n))

    part.colors = [color] * len(part.tris)
    part.fix_winding()
    return part


def ellipsoid(rx, ry, rz, color, segments=10, rings=6):
    """だ円体(中心は原点)"""
    secs = []
    for i in range(1, rings):
        phi = math.pi * i / rings
        secs.append((-ry * math.cos(phi), rx * math.sin(phi), rz * math.sin(phi), 0.0, 0.0))
    # 両端をすぼめて丸く閉じる
    secs = [(-ry, rx * 0.02, rz * 0.02, 0.0, 0.0)] + secs + [(ry, rx * 0.02, rz * 0.02, 0.0, 0.0)]
    return loft(secs, segments, color, axis="y", subdiv=1)


class Model:
    def __init__(self, name):
        self.name = name
        self.parts = []

    def add(self, part):
        part.fix_winding()
        self.parts.append(part)
        return part

    def add_pair(self, make):
        """左右対称のパーツを2つ追加する。make(side) の side は -1(左) か +1(右)"""
        for side in (-1.0, 1.0):
            self.add(make(side))

    def triangle_count(self):
        return sum(len(p.tris) for p in self.parts)

    def write_obj(self, path):
        lines = [
            "# %s - procedural model for ActionEngine" % self.name,
            "# Generated by tools/make_models.py",
            "# front = +z, feet at y = 0, colors = Assets/Textures/palette.bmp",
            "# vertices: %d, triangles: %d" % (sum(len(p.verts) for p in self.parts), self.triangle_count()),
            "",
        ]
        for i in range(PALETTE_SLOTS):
            lines.append("vt %.5f %.5f" % palette_uv(i))
        lines.append("")

        offset = 0
        for part in self.parts:
            # なめらかな法線(頂点のまわりの面の法線を、面積の重みつきで平均)
            normals = [(0.0, 0.0, 0.0)] * len(part.verts)
            for a, b, c in part.tris:
                n = cross(sub(part.verts[b], part.verts[a]), sub(part.verts[c], part.verts[a]))
                for i in (a, b, c):
                    normals[i] = add(normals[i], n)
            for v in part.verts:
                lines.append("v %.4f %.4f %.4f" % v)
            for n in normals:
                lines.append("vn %.4f %.4f %.4f" % normalize(n))
            for (a, b, c), col in zip(part.tris, part.colors):
                lines.append("f " + " ".join(
                    "%d/%d/%d" % (offset + i + 1, col + 1, offset + i + 1) for i in (a, b, c)))
            offset += len(part.verts)

        with open(path, "w", newline="\n") as f:
            f.write("\n".join(lines) + "\n")


# ---------------------------------------------------------------------------
# ペンギン(高さ 約1.75)
# ---------------------------------------------------------------------------
def make_penguin():
    m = Model("Penguin")
    C = COLOR

    # --- 胴体と頭(下から上へ輪を積む) ---  (y, 横幅, 奥行き, 中心x, 中心z)
    body = loft([
        (0.05, 0.20, 0.18, 0.0, 0.00),
        (0.10, 0.36, 0.33, 0.0, 0.00),
        (0.30, 0.50, 0.47, 0.0, 0.02),
        (0.58, 0.55, 0.53, 0.0, 0.04),
        (0.85, 0.51, 0.48, 0.0, 0.03),
        (1.08, 0.41, 0.39, 0.0, 0.01),
        (1.22, 0.35, 0.34, 0.0, 0.01),
        (1.36, 0.37, 0.37, 0.0, 0.03),
        (1.50, 0.35, 0.36, 0.0, 0.03),
        (1.63, 0.27, 0.28, 0.0, 0.02),
        (1.72, 0.14, 0.15, 0.0, 0.01),
        (1.75, 0.03, 0.03, 0.0, 0.01),
    ], segments=28, color=C["penguin_back"], subdiv=2)

    def body_color(p, n):
        x, y, z = p
        ang = abs(math.degrees(math.atan2(x, z)))  # 正面(+z)から何度ずれているか
        # お腹の白:下は広く、のどに向かって細くなる
        if y < 1.27:
            if y < 0.95:
                half = 78.0 - 22.0 * max(0.0, (0.25 - y) / 0.25)
            else:
                half = 78.0 - 40.0 * (y - 0.95) / 0.32
            if ang < half:
                # 胸の上のほうは黄色
                if 1.00 < y < 1.20 and ang < half - 6.0:
                    return C["yellow"]
                return C["penguin_belly"]
        # 頭は濃い青
        if y > 1.26:
            return C["penguin_dark"]
        return C["penguin_back"]

    m.add(body.paint(body_color))

    # --- くちばし(前へ伸びる円すい。上がオレンジ、下が濃いオレンジ) ---
    beak = loft([
        (0.30, 0.115, 0.085, 0.0, 1.415),
        (0.40, 0.100, 0.070, 0.0, 1.410),
        (0.50, 0.065, 0.045, 0.0, 1.395),
        (0.58, 0.025, 0.018, 0.0, 1.378),
    ], segments=10, color=C["orange"], axis="z", subdiv=2)
    m.add(beak.paint(lambda p, n: C["orange_dark"] if n[1] < -0.25 else C["orange"]))

    # --- 目(白目 + 黒目) ---
    def eye(side):
        direction = normalize((side * 0.47, 0.10, 0.88))
        center = add((0.0, 1.47, 0.03), mul(direction, 0.335))
        return ellipsoid(0.070, 0.078, 0.045, C["eye_white"], segments=10, rings=6) \
            .transform(lambda v: rot_y(v, side * 28.0)).move(center)

    def pupil(side):
        direction = normalize((side * 0.47, 0.10, 0.88))
        center = add((0.0, 1.47, 0.03), mul(direction, 0.372))
        return ellipsoid(0.038, 0.044, 0.026, C["black"], segments=8, rings=4) \
            .transform(lambda v: rot_y(v, side * 28.0)).move(center)

    m.add_pair(eye)
    m.add_pair(pupil)

    # --- 翼(平たいだ円体を、肩から外へ少し開く) ---
    def flipper(side):
        part = loft([
            (0.00, 0.050, 0.10, 0.0, 0.00),
            (-0.08, 0.070, 0.17, 0.0, 0.00),
            (-0.25, 0.065, 0.19, 0.0, -0.01),
            (-0.45, 0.050, 0.15, 0.0, -0.03),
            (-0.60, 0.035, 0.09, 0.0, -0.05),
            (-0.67, 0.012, 0.03, 0.0, -0.06),
        ], segments=10, color=C["penguin_back"], subdiv=2)
        part.transform(lambda v: rot_z(v, side * 20.0)).move((side * 0.40, 1.12, 0.0))
        # 内側(体のほうを向いた面)は白、先は濃い青
        def col(p, n):
            if n[0] * side < -0.55:
                return C["penguin_belly"]
            return C["penguin_dark"] if p[1] < 0.62 else C["penguin_back"]
        return part.paint(col)

    m.add_pair(flipper)

    # --- 足(かかと + 水かき + 指3本) ---
    def foot_base(side):
        return loft([
            (-0.10, 0.060, 0.050, 0.0, 0.055),
            (0.02, 0.095, 0.060, 0.0, 0.062),
            (0.18, 0.140, 0.040, 0.0, 0.042),
            (0.32, 0.170, 0.022, 0.0, 0.024),
            (0.36, 0.150, 0.012, 0.0, 0.014),
        ], segments=8, color=C["orange"], axis="z", subdiv=2) \
            .transform(lambda v: rot_y(v, side * 12.0)).move((side * 0.21, 0.0, 0.10))

    m.add_pair(foot_base)
    for toe_deg in (-24.0, 0.0, 24.0):
        def toe(side, toe_deg=toe_deg):
            part = ellipsoid(0.042, 0.034, 0.13, C["orange"], segments=6, rings=4)
            part.move((0.0, 0.036, 0.30))
            part.transform(lambda v: rot_y(v, toe_deg + side * 12.0))
            return part.move((side * 0.21, 0.0, 0.10)) \
                .paint(lambda p, n: C["orange_dark"] if p[2] > 0.44 else C["orange"])
        m.add_pair(toe)

    # --- しっぽ ---
    tail = loft([
        (0.00, 0.16, 0.070, 0.0, 0.0),
        (0.12, 0.11, 0.050, 0.0, 0.0),
        (0.24, 0.02, 0.012, 0.0, 0.0),
    ], segments=8, color=C["penguin_dark"], axis="z", subdiv=2)
    tail.transform(lambda v: rot_x((v[0], v[1], -v[2]), 28.0)).move((0.0, 0.20, -0.40))
    m.add(tail.paint(lambda p, n: C["penguin_dark"]))

    return m


# ---------------------------------------------------------------------------
# シロクマ(体長 約3.7、肩の高さ 約1.8)
# ---------------------------------------------------------------------------
def make_polar_bear():
    m = Model("Polar Bear")
    C = COLOR

    # --- 胴体〜首〜頭〜鼻先(おしりから前へ輪を積む) --- (z, 横幅, 縦幅, 中心x, 中心y)
    body = loft([
        (-1.50, 0.10, 0.12, 0.0, 1.14),
        (-1.42, 0.36, 0.40, 0.0, 1.14),
        (-1.15, 0.58, 0.60, 0.0, 1.15),
        (-0.75, 0.66, 0.66, 0.0, 1.15),
        (-0.20, 0.64, 0.62, 0.0, 1.12),
        (0.35, 0.62, 0.66, 0.0, 1.17),
        (0.75, 0.52, 0.58, 0.0, 1.24),
        (1.05, 0.40, 0.44, 0.0, 1.30),
        (1.28, 0.34, 0.35, 0.0, 1.35),
        (1.48, 0.37, 0.36, 0.0, 1.39),
        (1.68, 0.34, 0.32, 0.0, 1.37),
        (1.84, 0.24, 0.22, 0.0, 1.30),
        (2.00, 0.18, 0.165, 0.0, 1.25),
        (2.15, 0.155, 0.14, 0.0, 1.22),
        (2.23, 0.11, 0.10, 0.0, 1.21),
        (2.26, 0.04, 0.04, 0.0, 1.21),
    ], segments=20, color=C["bear_fur"], axis="z", subdiv=2)
    # 鼻先(マズル)だけ、少し色を落とす
    m.add(body.paint(lambda p, n: C["bear_shade"] if p[2] > 1.93 else C["bear_fur"]))

    # --- 鼻 ---
    m.add(ellipsoid(0.085, 0.060, 0.060, C["black"], segments=10, rings=6).move((0.0, 1.265, 2.235)))

    # --- 目 ---
    m.add_pair(lambda side: ellipsoid(0.040, 0.045, 0.030, C["black"], segments=8, rings=5)
               .transform(lambda v: rot_y(v, side * 38.0)).move((side * 0.215, 1.50, 1.74)))

    # --- 耳(外側は毛の色、前を向いた内側は灰色) ---
    def ear(side):
        part = ellipsoid(0.115, 0.115, 0.055, C["bear_fur"], segments=10, rings=6)
        part.transform(lambda v: rot_y(rot_z(v, side * -18.0), side * 15.0)).move((side * 0.27, 1.71, 1.44))
        return part.paint(lambda p, n: C["dark_grey"]
                          if n[2] > 0.80 and abs(p[0] - side * 0.27) < 0.075 and p[1] < 1.78
                          else C["bear_fur"])

    m.add_pair(ear)

    # --- 脚(上から下へ。足先は前に張り出す) --- (y, 横幅, 奥行き, 中心x, 中心z)
    def leg(x, z, thigh):
        def make(side):
            part = loft([
                (1.25, 0.17, 0.26 * thigh, 0.0, 0.00),
                (0.95, 0.25 * thigh, 0.34 * thigh, 0.0, 0.00),
                (0.65, 0.23, 0.27, 0.0, -0.02),
                (0.38, 0.20, 0.22, 0.0, -0.02),
                (0.20, 0.20, 0.23, 0.0, 0.02),
                (0.10, 0.23, 0.30, 0.0, 0.08),
                (0.03, 0.22, 0.30, 0.0, 0.09),
                (0.00, 0.17, 0.24, 0.0, 0.09),
            ], segments=12, color=C["bear_fur"], subdiv=2).move((side * x, 0.0, z))
            return part.paint(lambda p, n: C["bear_shade"] if p[1] < 0.16 else C["bear_fur"])
        return make

    front_x, front_z = 0.37, 0.42
    hind_x, hind_z = 0.37, -0.92
    m.add_pair(leg(front_x, front_z, 1.00))
    m.add_pair(leg(hind_x, hind_z, 1.18))

    # --- 爪(足先に3本ずつ) ---
    for lx, lz in ((front_x, front_z), (hind_x, hind_z)):
        for dx in (-0.12, 0.0, 0.12):
            def claw(side, lx=lx, lz=lz, dx=dx):
                forward = 0.385 - abs(dx) * 0.35
                return ellipsoid(0.028, 0.026, 0.055, C["dark_grey"], segments=5, rings=3) \
                    .move((side * lx + dx, 0.045, lz + forward))
            m.add_pair(claw)

    # --- しっぽ ---
    m.add(ellipsoid(0.13, 0.12, 0.14, C["bear_fur"], segments=10, rings=6).move((0.0, 1.27, -1.50)))

    return m


# ---------------------------------------------------------------------------
if __name__ == "__main__":
    os.makedirs("Assets/Models", exist_ok=True)
    os.makedirs("Assets/Textures", exist_ok=True)

    write_palette_bmp("Assets/Textures/palette.bmp")
    print("Assets/Textures/palette.bmp")

    for filename, model in (("penguin_blue.obj", make_penguin()), ("polar_bear.obj", make_polar_bear())):
        path = os.path.join("Assets/Models", filename)
        model.write_obj(path)
        print("%s  (%d triangles)" % (path, model.triangle_count()))
