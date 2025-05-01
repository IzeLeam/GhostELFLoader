#pragma once

int decrypt_library(const char *encrypted_path, const char *key);
void encrypt_file(const char *input_path, const char *output_path, const char *key);