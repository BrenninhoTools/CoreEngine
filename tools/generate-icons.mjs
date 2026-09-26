import { deflateSync } from "node:zlib";
import { mkdirSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const root = resolve(dirname(fileURLToPath(import.meta.url)), "..");

const crcTable = new Uint32Array(256).map((_, n) => {
  let c = n;
  for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
  return c >>> 0;
});

const crc32 = (buf) => {
  let c = 0xffffffff;
  for (const byte of buf) c = crcTable[(c ^ byte) & 0xff] ^ (c >>> 8);
  return (c ^ 0xffffffff) >>> 0;
};

const chunk = (type, data) => {
  const out = Buffer.alloc(12 + data.length);
  out.writeUInt32BE(data.length, 0);
  out.write(type, 4, "ascii");
  data.copy(out, 8);
  out.writeUInt32BE(crc32(out.subarray(4, 8 + data.length)), 8 + data.length);
  return out;
};

const encodePng = (size, rgba) => {
  const header = Buffer.alloc(13);
  header.writeUInt32BE(size, 0);
  header.writeUInt32BE(size, 4);
  header[8] = 8;
  header[9] = 6;
  const stride = size * 4;
  const raw = Buffer.alloc((stride + 1) * size);
  for (let y = 0; y < size; y++) {
    raw[y * (stride + 1)] = 0;
    rgba.copy(raw, y * (stride + 1) + 1, y * stride, (y + 1) * stride);
  }
  return Buffer.concat([
    Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]),
    chunk("IHDR", header),
    chunk("IDAT", deflateSync(raw, { level: 9 })),
    chunk("IEND", Buffer.alloc(0)),
  ]);
};

const clamp01 = (v) => (v < 0 ? 0 : v > 1 ? 1 : v);
const mix = (a, b, t) => a + (b - a) * t;
const mixColor = (a, b, t) => [mix(a[0], b[0], t), mix(a[1], b[1], t), mix(a[2], b[2], t)];

const backgroundTop = [30, 41, 82];
const backgroundBottom = [7, 9, 22];
const accent = [56, 189, 248];
const highlight = [236, 250, 255];

const roundedBoxDistance = (px, py, half, radius) => {
  const qx = Math.abs(px) - (half - radius);
  const qy = Math.abs(py) - (half - radius);
  const ox = Math.max(qx, 0);
  const oy = Math.max(qy, 0);
  return Math.hypot(ox, oy) + Math.min(Math.max(qx, qy), 0) - radius;
};

const render = (size, rounded) => {
  const out = Buffer.alloc(size * size * 4);
  const half = size / 2;
  const radius = size * 0.225;
  const ringRadius = size * 0.29;
  const ringWidth = size * 0.055;
  const discRadius = size * 0.145;
  const orbitAngle = -Math.PI / 4;
  const dotX = Math.cos(orbitAngle) * ringRadius;
  const dotY = Math.sin(orbitAngle) * ringRadius;
  const dotRadius = size * 0.048;
  const glowScale = size * 0.05;

  for (let y = 0; y < size; y++) {
    for (let x = 0; x < size; x++) {
      const px = x + 0.5 - half;
      const py = y + 0.5 - half;
      const coverage = rounded ? clamp01(0.5 - roundedBoxDistance(px, py, half, radius)) : 1;

      let color = mixColor(backgroundTop, backgroundBottom, clamp01((py + half) / size));

      const dist = Math.hypot(px, py);
      const ringDist = Math.abs(dist - ringRadius) - ringWidth / 2;
      const glow = Math.exp(-Math.max(ringDist, 0) / glowScale) * 0.35;
      color = mixColor(color, accent, glow);

      color = mixColor(color, accent, clamp01(0.5 - ringDist));

      const discDist = dist - discRadius;
      const discColor = mixColor(highlight, accent, clamp01(dist / discRadius) * 0.6);
      color = mixColor(color, discColor, clamp01(0.5 - discDist));

      const dotDist = Math.hypot(px - dotX, py - dotY) - dotRadius;
      color = mixColor(color, highlight, clamp01(0.5 - dotDist));

      const i = (y * size + x) * 4;
      out[i] = Math.round(color[0]);
      out[i + 1] = Math.round(color[1]);
      out[i + 2] = Math.round(color[2]);
      out[i + 3] = Math.round(coverage * 255);
    }
  }
  return out;
};

const pngCache = new Map();
const png = (size, rounded = true) => {
  const key = `${size}:${rounded}`;
  if (!pngCache.has(key)) pngCache.set(key, encodePng(size, render(size, rounded)));
  return pngCache.get(key);
};

const write = (path, data) => {
  const full = join(root, path);
  mkdirSync(dirname(full), { recursive: true });
  writeFileSync(full, data);
};

const encodeIco = (sizes) => {
  const images = sizes.map((size) => png(size));
  const header = Buffer.alloc(6);
  header.writeUInt16LE(1, 2);
  header.writeUInt16LE(images.length, 4);
  const entries = Buffer.alloc(16 * images.length);
  let offset = header.length + entries.length;
  images.forEach((image, index) => {
    const size = sizes[index];
    const at = index * 16;
    entries[at] = size >= 256 ? 0 : size;
    entries[at + 1] = size >= 256 ? 0 : size;
    entries.writeUInt16LE(1, at + 4);
    entries.writeUInt16LE(32, at + 6);
    entries.writeUInt32LE(image.length, at + 8);
    entries.writeUInt32LE(offset, at + 12);
    offset += image.length;
  });
  return Buffer.concat([header, entries, ...images]);
};

const encodeIcns = (variants) => {
  const parts = variants.map(([type, size]) => {
    const data = png(size);
    const head = Buffer.alloc(8);
    head.write(type, 0, "ascii");
    head.writeUInt32BE(data.length + 8, 4);
    return Buffer.concat([head, data]);
  });
  const body = Buffer.concat(parts);
  const head = Buffer.alloc(8);
  head.write("icns", 0, "ascii");
  head.writeUInt32BE(body.length + 8, 4);
  return Buffer.concat([head, body]);
};

const cArray = (bytes) => {
  const lines = [];
  for (let i = 0; i < bytes.length; i += 24) {
    lines.push("    " + Array.from(bytes.subarray(i, i + 24), (b) => `0x${b.toString(16).padStart(2, "0")}`).join(", ") + ",");
  }
  return lines.join("\n");
};

write("assets/icon/icon.png", png(1024));
write("assets/icon/icon.ico", encodeIco([16, 32, 48, 64, 128, 256]));
write("assets/icon/icon.icns", encodeIcns([
  ["ic07", 128],
  ["ic08", 256],
  ["ic09", 512],
  ["ic10", 1024],
]));

const androidDensities = { mdpi: 48, hdpi: 72, xhdpi: 96, xxhdpi: 144, xxxhdpi: 192 };
for (const [density, size] of Object.entries(androidDensities)) {
  write(`android/app/src/main/res/mipmap-${density}/ic_launcher.png`, png(size));
}

const iosSet = "platform/ios/Assets.xcassets/AppIcon.appiconset";
write(`${iosSet}/icon-1024.png`, png(1024, false));
write(`${iosSet}/Contents.json`, JSON.stringify({
  images: [{ filename: "icon-1024.png", idiom: "universal", platform: "ios", size: "1024x1024" }],
  info: { author: "xcode", version: 1 },
}, null, 2) + "\n");
write("platform/ios/Assets.xcassets/Contents.json", JSON.stringify({
  info: { author: "xcode", version: 1 },
}, null, 2) + "\n");

const embeddedSize = 64;
const embedded = render(embeddedSize, true);
write("engine/src/EmbeddedIcon.h", [
  "#pragma once",
  "",
  "namespace core::detail {",
  "",
  `inline constexpr int kEmbeddedIconSize = ${embeddedSize};`,
  "",
  "inline constexpr unsigned char kEmbeddedIconPixels[] = {",
  cArray(embedded),
  "};",
  "",
  "}",
  "",
].join("\n"));
