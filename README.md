# 🧠 ISOS Loader – Advanced Dynamic ELF Loader with In-Memory Decryption & Obfuscation

ISOS Loader is a **custom dynamic ELF loader** designed to demonstrate advanced low-level programming concepts such as binary loading, symbol resolution, runtime decryption, and anti-static analysis techniques.

This project showcases deep expertise in:
- ELF internals & memory mapping
- Custom PLT/GOT mechanics
- In-memory loading with runtime decryption
- Runtime obfuscation to protect shared libraries

---

## 🚀 Features

### 🔧 Custom ELF Loader
- Parses ELF headers & program segments manually.
- Loads segments into memory and applies correct memory protections.
- Applies relocations and resolves both imported and exported symbols dynamically.

### 🧮 Custom PLT/GOT Implementation
- Implements its own Procedure Linkage Table (PLT) and Global Offset Table (GOT).
- Caches resolved symbol addresses in the GOT for optimized repeated function calls.
- Fully bypasses standard dynamic linker (`ld.so`).

### 🛡 Runtime Library Encryption / Decryption
- Encrypts shared libraries using **XOR encryption** (fast & simple).
- Decrypts libraries **directly into memory** using `memfd_create()` for stealth.
- Supports secure passing of decryption key via command-line.

### 🕵️ Obfuscation & Anti-Analysis
- Entire libraries are **never stored unencrypted on disk**.
- Prevents static analysis by decrypting at runtime only.
- Leaves **zero trace** of the actual shared object on the file system.

### 🖥 Command-Line Interface (CLI)
- Load encrypted or plaintext ELF shared libraries.
- Specify one or more functions to call dynamically.
- Enable verbose output for step-by-step debug information.

---

## ⚙️ How It Works

### 🔐 1. Library Encryption
A shared object (`.so`) is XOR-encrypted with a secret key:
```bash
./encrypt lib/libfoo.so lib/libfoo.so.enc "my_secret_key"
```

### 🧠 2. Custom ELF Loading
At runtime:
- The encrypted `.so` is decrypted into memory.
- The loader parses the ELF structure, loads segments, and performs relocations.
- A custom PLT/GOT is used to resolve symbols.

### ⚠️ 3. Runtime Execution
- Functions specified via command line are resolved and called.
- No trace is left on disk thanks to `memfd_create()`.

---

## 🛠 Usage

### 🔒 Encrypt a Shared Library
```bash
./encrypt lib/libfoo.so lib/libfoo.so.enc "my_secret_key"
```

### ▶️ Run the Loader
```bash
./isos_loader -k "my_secret_key" lib/libfoo.so.enc foo_exported bar_exported
```

### 🐞 Verbose Mode (Debugging)
```bash
./isos_loader -v -k "my_secret_key" lib/libfoo.so.enc foo_exported bar_exported
```

---

## 🎯 Demonstrated Skills

### 🧬 Low-Level Programming
- Manual ELF parsing & segment mapping
- Dynamic relocation & memory protections
- PLT/GOT resolution and symbol caching

### 🛡 Security & Obfuscation
- Runtime XOR encryption/decryption
- In-memory execution using `memfd_create()`
- Anti-static analysis: no decrypted code stored on disk

### 🧰 Tooling & Architecture
- Custom CLI with secure argument handling
- Debug-friendly verbose mode
- Modular design and clean separation of concerns

---

## 📜 License

This project is licensed under the **MIT License**.  
See the [LICENSE](./LICENSE) file for full details.

---

## 🙏 Acknowledgments

Special thanks to the project referents whose resources and insights helped bring this project to life.

> 🧠 **Disclaimer:** This project is intended for **educational and research purposes only**.  
> Do not use these techniques for malicious purposes.

---

Made with ❤️ by Luca Pourceau.
