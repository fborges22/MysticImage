#!/usr/bin/env python3
"""Losslessly convert files to PNG images and hide files inside PNG images."""

from __future__ import annotations

import argparse
import math
import struct
import sys
from pathlib import Path

try:
    from PIL import Image, UnidentifiedImageError
except ImportError as error:  # pragma: no cover - exercised only without dependencies
    raise SystemExit(
        "Pillow is required. Install it with: python -m pip install -r requirements.txt"
    ) from error

STANDARD_MAGIC = b"MYSTIC01"
STEGO_MAGIC = b"MYSTEG01"
HEADER = struct.Struct(">8sQ")


class MysticImageError(Exception):
    """An expected conversion or validation failure."""


def _read(path: Path) -> bytes:
    try:
        return path.read_bytes()
    except OSError as error:
        raise MysticImageError(f"Could not read {path}: {error}") from error


def _write(path: Path, data: bytes) -> None:
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
    except OSError as error:
        raise MysticImageError(f"Could not write {path}: {error}") from error


def _open_rgb(path: Path) -> Image.Image:
    try:
        with Image.open(path) as source:
            return source.convert("RGB")
    except (OSError, UnidentifiedImageError) as error:
        raise MysticImageError(f"Could not open image {path}: {error}") from error


def _save_png(image: Image.Image, path: Path) -> None:
    if path.suffix.lower() != ".png":
        raise MysticImageError("Output must use the lossless .png format")
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        image.save(path, format="PNG", optimize=True)
    except OSError as error:
        raise MysticImageError(f"Could not save {path}: {error}") from error


def binary_file_to_image(binary_file_path: str | Path, output_image_path: str | Path) -> None:
    payload = _read(Path(binary_file_path))
    encoded = HEADER.pack(STANDARD_MAGIC, len(payload)) + payload
    pixel_count = math.ceil(len(encoded) / 3)
    width = math.ceil(math.sqrt(pixel_count))
    height = math.ceil(pixel_count / width)
    encoded += bytes(width * height * 3 - len(encoded))
    image = Image.frombytes("RGB", (width, height), encoded)
    _save_png(image, Path(output_image_path))


def image_to_binary_file(image_file_path: str | Path, output_binary_file_path: str | Path) -> None:
    raw = _open_rgb(Path(image_file_path)).tobytes()
    if len(raw) < HEADER.size:
        raise MysticImageError("Image is too small to contain a MysticImage header")
    magic, length = HEADER.unpack_from(raw)
    if magic != STANDARD_MAGIC:
        raise MysticImageError("Image is not a MysticImage file (invalid header)")
    if length > len(raw) - HEADER.size:
        raise MysticImageError("Image is corrupt: payload length exceeds its capacity")
    _write(Path(output_binary_file_path), raw[HEADER.size : HEADER.size + length])


def hide_file(payload_path: str | Path, carrier_path: str | Path, output_path: str | Path) -> None:
    payload = _read(Path(payload_path))
    encoded = HEADER.pack(STEGO_MAGIC, len(payload)) + payload
    carrier = _open_rgb(Path(carrier_path))
    pixels = bytearray(carrier.tobytes())
    required = len(encoded) * 8
    if required > len(pixels):
        capacity = max(0, len(pixels) // 8 - HEADER.size)
        raise MysticImageError(
            f"Payload is too large: carrier capacity is {capacity} bytes, got {len(payload)}"
        )
    for bit_offset in range(required):
        bit = (encoded[bit_offset // 8] >> (7 - bit_offset % 8)) & 1
        pixels[bit_offset] = (pixels[bit_offset] & 0xFE) | bit
    _save_png(Image.frombytes("RGB", carrier.size, bytes(pixels)), Path(output_path))


def reveal_file(image_path: str | Path, output_path: str | Path) -> None:
    pixels = _open_rgb(Path(image_path)).tobytes()

    def extract(byte_count: int) -> bytes:
        result = bytearray(byte_count)
        for bit_offset in range(byte_count * 8):
            result[bit_offset // 8] |= (pixels[bit_offset] & 1) << (7 - bit_offset % 8)
        return bytes(result)

    capacity = len(pixels) // 8
    if capacity < HEADER.size:
        raise MysticImageError("Image is too small to contain a hidden payload")
    magic, length = HEADER.unpack(extract(HEADER.size))
    if magic != STEGO_MAGIC:
        raise MysticImageError("Image does not contain a MysticImage steganographic header")
    if length > capacity - HEADER.size:
        raise MysticImageError("Image is corrupt: hidden payload length exceeds its capacity")
    encoded = extract(HEADER.size + length)
    _write(Path(output_path), encoded[HEADER.size:])


def convert_directory(input_dir: Path, output_dir: Path, mode: str, overwrite: bool) -> int:
    if not input_dir.is_dir():
        raise MysticImageError(f"Input directory does not exist: {input_dir}")
    output_dir.mkdir(parents=True, exist_ok=True)
    inputs = sorted(input_dir.glob("*.png")) if mode == "png2bin" else sorted(input_dir.iterdir())
    inputs = [path for path in inputs if path.is_file()]
    converted = 0
    for source in inputs:
        target = output_dir / (source.stem + (".bin" if mode == "png2bin" else ".png"))
        if target.exists() and not overwrite:
            print(f"Skipping existing file: {target}", file=sys.stderr)
            continue
        (image_to_binary_file if mode == "png2bin" else binary_file_to_image)(source, target)
        converted += 1
    return converted


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    for command in ("bin2png", "png2bin"):
        conversion = subparsers.add_parser(command)
        conversion.add_argument("input", type=Path)
        conversion.add_argument("output", type=Path)
        conversion.add_argument("--directory", action="store_true", help="convert a directory")
        conversion.add_argument("--overwrite", action="store_true")
    hide = subparsers.add_parser("hide", help="hide a payload in a lossless carrier image")
    hide.add_argument("payload", type=Path)
    hide.add_argument("carrier", type=Path)
    hide.add_argument("output", type=Path)
    hide.add_argument("--overwrite", action="store_true")
    reveal = subparsers.add_parser("reveal", help="extract a hidden payload")
    reveal.add_argument("input", type=Path)
    reveal.add_argument("output", type=Path)
    reveal.add_argument("--overwrite", action="store_true")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        output = args.output
        if output.exists() and not args.overwrite and not getattr(args, "directory", False):
            raise MysticImageError(f"Output already exists (use --overwrite): {output}")
        if args.command in ("bin2png", "png2bin"):
            if args.directory:
                count = convert_directory(args.input, output, args.command, args.overwrite)
                print(f"Converted {count} file(s)")
            else:
                (image_to_binary_file if args.command == "png2bin" else binary_file_to_image)(
                    args.input, output
                )
                print(f"Created {output}")
        elif args.command == "hide":
            hide_file(args.payload, args.carrier, output)
            print(f"Created {output}")
        else:
            reveal_file(args.input, output)
            print(f"Created {output}")
    except MysticImageError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
