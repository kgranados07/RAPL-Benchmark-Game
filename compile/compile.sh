#!/bin/bash

set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BENCHMARKS="$ROOT/benchmarks"
BIN="$ROOT/bin"

mkdir -p "$BIN"

echo "Compiling benchmarks..."

find "$BENCHMARKS" -type f | while read -r file
do
    benchmark=$(basename "$(dirname "$file")")
    filename=$(basename "$file")
    extension="${filename##*.}"
    basename="${filename%.*}"

    outdir="$BIN/$benchmark"
    mkdir -p "$outdir"

    case "$extension" in

        c)
            echo "C: $benchmark/$filename"
            gcc -O3 "$file" -o "$outdir/${basename}_c"
            ;;

        cpp|cc|cxx)
            echo "C++: $benchmark/$filename"
            g++ -O3 "$file" -o "$outdir/${basename}_cpp"
            ;;

        cs)
            echo "C#: $benchmark/$filename"
            # Added --property:AllowUnsafeBlocks=true for dotnet and -unsafe for csc fallback
            dotnet build -c Release "$file" -o "$outdir" --property:AllowUnsafeBlocks=true || csc -out:"$outdir/${basename}_cs.exe" -unsafe "$file"
            ;;

        rs)
            echo "Rust: $benchmark/$filename"
            # -O maps to opt-level=2. We add target-cpu=native and codegen-units=1 for maximum performance.
            rustc -O -C target-cpu=native -C codegen-units=1 "$file" -o "$outdir/${basename}_rust"
            ;;

        go)
            echo "Go: $benchmark/$filename"
            go build -o "$outdir/${basename}_go" "$file"
            ;;

        java)
            echo "Java: $benchmark/$filename"
            javac -d "$outdir" "$file"
            ;;

        scala)
            echo "Scala: $benchmark/$filename"
            scalac -d "$outdir" "$file"
            ;;

        hs)
            echo "Haskell: $benchmark/$filename"
            ghc -O2 "$file" -o "$outdir/${basename}_hs" -outputdir "$outdir"
            ;;

        ml)
            echo "OCaml: $benchmark/$filename"
            ocamlopt -o "$outdir/${basename}_ocaml" "$file"
            ;;

        py)
            echo "Python: $benchmark/$filename (no compilation)"
            ;;

        rb)
            echo "Ruby: $benchmark/$filename (no compilation)"
            ;;

        js)
            echo "JavaScript: $benchmark/$filename (no compilation)"
            ;;

        lua)
            echo "Lua: $benchmark/$filename (no compilation)"
            ;;

        php)
            echo "PHP: $benchmark/$filename (no compilation)"
            ;;

        *)
            echo "Skipping unknown file: $filename"
            ;;

    esac

done

echo "Done."