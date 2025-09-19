# GhostELF - Dynamic ELF Loader

Design of a dynamic ELF loader enabling the encrypted injection of a shared library into an ELF file, followed by the invisible execution of this code.

GhostELF is a custom dynamic ELF loader for offensive security research. It demonstrates controlled in-memory loading and execution of shared libraries, including runtime decryption, symbol resolution, and stealth techniques that avoid leaving decrypted artifacts on disk.

> This project is intended solely for educational and research purposes within authorized environments. Users must ensure compliance with all relevant laws, regulations and ethical guidelines. The authors disclaim any responsibility for misuse or unintended consequences arising from the use of this software.

## Overview

GhostELF implements the following capabilities:

- Manual ELF parsing and program segment mapping
- Correct memory protections per segment and relocation handling
- Custom PLT/GOT for symbol resolution and caching without `ld.so`
- Runtime decryption of shared objects into anonymous memory
- No plaintext `.so` persisted on disk during execution
- Verbose diagnostics for step-by-step tracing

## Architecture

1. Encryption stage: a shared object is encrypted with a symmetric XOR key.
2. Load stage: the encrypted payload is decrypted directly into memory and never written to disk.
3. Relocation and resolution: ELF segments are mapped, relocations applied, and symbols resolved via a custom PLT/GOT.
4. Invocation: exported functions are located and invoked based on CLI arguments.

## Quick start

Build targets are defined by the project `Makefile` in the repository root and `lib/Makefile`.

### Encrypt a shared library
```bash
./encrypt lib/libfoo.so lib/libfoo.so.enc "my_secret_key"
```

### Run the loader against the encrypted library
```bash
./isos_loader -k "my_secret_key" lib/libfoo.so.enc foo_exported bar_exported
```

### Enable verbose output
```bash
./isos_loader -v -k "my_secret_key" lib/libfoo.so.enc foo_exported bar_exported
```

Notes:
- The binaries `encrypt` and `isos_loader` are produced by the provided Makefiles.
- Function names (`foo_exported`, etc.) must exist in the target shared object.

## Implementation details

- ELF: Parses headers, program headers, and maps PT_LOAD segments with appropriate `PROT_*` flags.
- Relocations: Handles common relocation types for supported architectures (see `include/arch/`).
- Symbol resolution: Maintains a custom GOT; first resolution goes via a resolver, subsequent calls hit the cached address.
- In-memory payload: Decryption occurs into an anonymous file descriptor ; the plaintext never touches disk.

## Limitations

- XOR encryption is for demonstration only and provides minimal cryptographic strength.
- Supported relocation types and architectures are limited to those implemented in `include/arch/` and `src/`.
- Requires Linux with support for `memfd_create()` and standard ELF semantics.

## Legal and ethical notice

This software is provided for research and educational use in environments where you have explicit permission. You are responsible for complying with applicable laws and agreements. The authors and contributors disclaim liability for misuse.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Acknowledgments

Thanks to the project referents and the broader security research community for insights informing this work.
