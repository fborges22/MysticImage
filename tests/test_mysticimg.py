import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import mysticimg


class MysticImageTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def test_standard_round_trip_random_and_empty_files(self):
        for index, data in enumerate((b"", b"abc", os.urandom(4097))):
            source = self.root / f"source-{index}.bin"
            image = self.root / f"encoded-{index}.png"
            restored = self.root / f"restored-{index}.bin"
            source.write_bytes(data)
            mysticimg.binary_file_to_image(source, image)
            mysticimg.image_to_binary_file(image, restored)
            self.assertEqual(data, restored.read_bytes())

    def test_steganographic_round_trip_preserves_size_and_payload(self):
        carrier = self.root / "carrier.png"
        Image.new("RGB", (80, 80), (10, 20, 30)).save(carrier)
        payload = self.root / "payload.bin"
        payload.write_bytes(os.urandom(500))
        concealed = self.root / "concealed.png"
        restored = self.root / "restored.bin"
        mysticimg.hide_file(payload, carrier, concealed)
        mysticimg.reveal_file(concealed, restored)
        with Image.open(carrier) as carrier_image, Image.open(concealed) as concealed_image:
            self.assertEqual(carrier_image.size, concealed_image.size)
        self.assertEqual(payload.read_bytes(), restored.read_bytes())

    def test_rejects_invalid_standard_image(self):
        image = self.root / "ordinary.png"
        Image.new("RGB", (10, 10), "white").save(image)
        with self.assertRaisesRegex(mysticimg.MysticImageError, "invalid header"):
            mysticimg.image_to_binary_file(image, self.root / "output.bin")

    def test_rejects_oversized_steganographic_payload(self):
        carrier = self.root / "tiny.png"
        Image.new("RGB", (4, 4), "black").save(carrier)
        payload = self.root / "payload.bin"
        payload.write_bytes(b"x")
        with self.assertRaisesRegex(mysticimg.MysticImageError, "too large"):
            mysticimg.hide_file(payload, carrier, self.root / "output.png")

    def test_cli_protects_existing_output(self):
        source = self.root / "source.bin"
        output = self.root / "output.png"
        source.write_bytes(b"new")
        output.write_bytes(b"existing")
        result = subprocess.run(
            [sys.executable, "mysticimg.py", "bin2png", str(source), str(output)],
            cwd=Path(__file__).resolve().parents[1], capture_output=True, text=True,
        )
        self.assertEqual(1, result.returncode)
        self.assertEqual(b"existing", output.read_bytes())

    def test_cli_round_trip(self):
        source = self.root / "source.bin"
        encoded = self.root / "encoded.png"
        restored = self.root / "restored.bin"
        source.write_bytes(os.urandom(257))
        project = Path(__file__).resolve().parents[1]
        for arguments in (
            ("bin2png", source, encoded),
            ("png2bin", encoded, restored),
        ):
            result = subprocess.run(
                [sys.executable, "mysticimg.py", *(str(item) for item in arguments)],
                cwd=project, capture_output=True, text=True,
            )
            self.assertEqual(0, result.returncode, result.stderr)
        self.assertEqual(source.read_bytes(), restored.read_bytes())


if __name__ == "__main__":
    unittest.main()
