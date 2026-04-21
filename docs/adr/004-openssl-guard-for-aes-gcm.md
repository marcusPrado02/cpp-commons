# ADR-004: OpenSSL-guarded AES-GCM crypto

**Status:** Accepted  
**Date:** 2026-01-15

## Context

`AesGcmProvider` and `KeyRotationService` require AES-256-GCM encryption. This requires a crypto library. Options:

- **OpenSSL**: industry standard, installed on every production Linux, full FIPS support.
- **libsodium**: modern, safer API (`crypto_aead_aes256gcm`), but less universally installed.
- **Compile-time guard**: build the crypto code only when OpenSSL headers are present.

## Decision

Implement AES-GCM via OpenSSL EVP API, behind a `#ifdef CPP_COMMONS_HAS_OPENSSL` compile-time guard. CMake detects OpenSSL with `find_package(OpenSSL OPTIONAL_COMPONENTS Crypto)` and sets the flag automatically.

## Rationale

1. **Optional dependency.** Many consumers of cpp-commons don't need encryption (they use only `Result`, `UUID`, HTTP middleware). Requiring OpenSSL unconditionally would force them to install a crypto library for zero benefit.
2. **Ubiquity.** When needed, OpenSSL is already present on virtually every deployment target (Ubuntu, Alpine, Debian). Installing libsodium requires an extra apt/apk step.
3. **FIPS compliance.** Some regulated environments require FIPS-validated crypto. OpenSSL 3.x has a FIPS provider; libsodium does not.
4. **Graceful degradation.** Without OpenSSL, `AesGcmProvider` and `KeyRotationService` are unavailable at compile time. The guard makes this explicit rather than silently linking a stub.

## Trade-offs

- `#ifdef` guards make the API surface conditional. Consumers must check `CPP_COMMONS_HAS_OPENSSL` before using crypto types.
- OpenSSL's EVP API is verbose compared to libsodium's one-call `crypto_aead` functions. Mitigated by wrapping in `AesGcmProvider`.
- OpenSSL RAND_bytes is used for IV generation — callers must not reuse IVs. This is documented but not enforced by the type system.

## Consequences

- `CMakeLists.txt` calls `find_package(OpenSSL OPTIONAL_COMPONENTS Crypto)` and sets `CPP_COMMONS_HAS_OPENSSL` accordingly.
- Crypto-dependent tests use `#ifdef CPP_COMMONS_HAS_OPENSSL` guards so CI passes even on minimal images without OpenSSL headers.
- The `cpp_commons::security` target links `OpenSSL::Crypto` only when found.
