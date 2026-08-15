#!/usr/bin/env python3
"""AES-128 encryption reference backed by cryptography.

Inputs and outputs use the conventional AES byte order: a 16-byte plaintext
block and a 16-byte key, both shown as 32-character hex strings in the CLI.
"""

from __future__ import annotations

import argparse

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes


def encrypt_block(plaintext: bytes, key: bytes) -> bytes:
    """Encrypt one 16-byte block with AES-128 ECB.

    ECB is used only to expose the raw AES block permutation for reference
    checking.  The function performs no padding and accepts exactly one block.
    """
    if len(plaintext) != 16:
        raise ValueError("AES plaintext block must be exactly 16 bytes")
    if len(key) != 16:
        raise ValueError("AES-128 key must be exactly 16 bytes")

    cipher = Cipher(algorithms.AES(key), modes.ECB())
    encryptor = cipher.encryptor()
    return encryptor.update(plaintext) + encryptor.finalize()


def encrypt_hex(plaintext_hex: str, key_hex: str) -> str:
    plaintext = bytes.fromhex(plaintext_hex)
    key = bytes.fromhex(key_hex)
    return encrypt_block(plaintext, key).hex()


def _self_test() -> None:
    vectors = [
        (
            "00000000000000000000000000000000",
            "00000000000000000000000000000000",
            "66e94bd4ef8a2c3b884cfa59ca342b2e",
        ),
        (
            "00112233445566778899aabbccddeeff",
            "000102030405060708090a0b0c0d0e0f",
            "69c4e0d86a7b0430d8cdb78070b4c55a",
        ),
    ]
    for plaintext, key, expected in vectors:
        actual = encrypt_hex(plaintext, key)
        if actual != expected:
            raise AssertionError(
                f"AES self-test failed: pt={plaintext} key={key} "
                f"expected={expected} actual={actual}"
            )


def main() -> int:
    parser = argparse.ArgumentParser(description="AES-128 reference encryptor")
    parser.add_argument("plaintext", nargs="?", help="16-byte plaintext as hex")
    parser.add_argument("key", nargs="?", help="16-byte key as hex")
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="run built-in known-answer tests",
    )
    args = parser.parse_args()

    if args.self_test:
        _self_test()
        print("AES-128 self-test passed")
        return 0

    if args.plaintext is None or args.key is None:
        parser.error("plaintext and key are required unless --self-test is used")

    print(encrypt_hex(args.plaintext, args.key))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
