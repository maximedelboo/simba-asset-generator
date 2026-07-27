CC ?= clang
CFLAGS := -O3 -Wall -Wextra -Werror -fvisibility=hidden

# Names deliberately follow SimbaEnv.FindPlugin's first, portable lookup form:
# `<name>.<shared-suffix>` plus `<name>.<shared-suffix>.aarch64` on ARM macOS.
MACOS_X64 := mapnative.dylib
MACOS_ARM64 := mapnative64.dylib.aarch64
LINUX_X64 := mapnative.so
LINUX_ARM64 := mapnative64.so.aarch64
WINDOWS_X64 := mapnative.dll

.PHONY: macos-x64 macos-arm64 linux-x64 linux-arm64 windows-x64 verify clean

macos-x64:
	$(CC) -arch x86_64 $(CFLAGS) -dynamiclib -o $(MACOS_X64) mapnative.c

macos-arm64:
	$(CC) -arch arm64 $(CFLAGS) -dynamiclib -o $(MACOS_ARM64) mapnative.c

# The Linux and Windows releases are built reproducibly by the GitHub workflow
# on their native runners. These local targets are convenient for maintainers
# with the matching compiler installed.
linux-x64:
	$(CC) $(CFLAGS) -shared -fPIC -o $(LINUX_X64) mapnative.c

linux-arm64:
	$(CC) $(CFLAGS) -shared -fPIC -o $(LINUX_ARM64) mapnative.c

windows-x64:
	$(CC) $(CFLAGS) -shared -o $(WINDOWS_X64) mapnative.c

verify:
	@for f in $(MACOS_X64) $(MACOS_ARM64) $(LINUX_X64) $(LINUX_ARM64) $(WINDOWS_X64); do test -f $$f || { echo "missing $$f" >&2; exit 1; }; done

clean:
	rm -f $(MACOS_X64) $(MACOS_ARM64) $(LINUX_X64) $(LINUX_ARM64) $(WINDOWS_X64)
