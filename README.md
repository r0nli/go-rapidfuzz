# go-rapidfuzz

A Go wrapper for the RapidFuzz fuzzy string matching library.

## Supported Platforms

- Linux AMD64
- Windows AMD64 
- ARM Darwin (Apple Silicon)

## Building

This project uses Task for building the C++ libraries. Install Task first:

```bash
go install github.com/go-task/task/v3/cmd/task@latest
```

### Build for specific platforms:

```bash
# Linux AMD64
task compile-lib-linux

# Windows AMD64 (requires mingw-w64)
task compile-lib-windows  

# ARM Darwin/Apple Silicon (requires clang or osxcross)
task compile-lib-darwin-arm64

# Build all supported platforms
task compile-all
```

### Platform-specific requirements:

- **Linux**: Standard GCC toolchain
- **Windows**: MinGW-w64 cross-compilation toolchain
- **ARM Darwin**: Either run on macOS with Xcode/clang, or use [osxcross](https://github.com/tpoechtrager/osxcross) on Linux

## Usage

```go
package main

import (
    "fmt"
    "github.com/r0nli/go-rapidfuzz"
)

func main() {
    choices := []string{"apple", "banana", "orange", "grape"}
    
    // Find best match
    match, score, found := rapidfuzz.ExtractOne("aple", choices, 0.0)
    if found {
        fmt.Printf("Best match: %s (score: %.2f)\n", match, score)
    }
    
    // Find all matches above threshold
    matches := rapidfuzz.Extract("aple", choices, 50.0)
    for _, m := range matches {
        fmt.Printf("Match %d: %s (score: %.2f)\n", m.Index, m.Match, m.Score)
    }
}
```