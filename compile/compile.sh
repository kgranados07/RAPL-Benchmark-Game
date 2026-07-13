#!/bin/bash

set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BENCHMARKS="$ROOT/benchmarks"
BIN="$ROOT/bin"

mkdir -p "$BIN"

need_cmd() {
    local cmd="$1"
    local label="$2"
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "Skipping $label ($cmd not found)"
        return 1
    fi
    return 0
}

copy_source() {
    local src="$1"
    local dest_dir="$2"
    cp "$src" "$dest_dir/$(basename "$src")"
}

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
            if need_cmd gcc "C"; then
                if ! gcc -O3 -fopenmp "$file" -o "$outdir/${basename}_c" -lm -lgmp; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        cpp|cc|cxx)
            echo "C++: $benchmark/$filename"
            if need_cmd g++ "C++"; then
                if ! g++ -O3 -fopenmp -std=c++17 -pthread "$file" -o "$outdir/${basename}_cpp" -lm -lgmp; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        cs)
            echo "C#: $benchmark/$filename"
            if need_cmd dotnet "C#"; then
                if ! dotnet build -c Release "$file" -o "$outdir" --property:AllowUnsafeBlocks=true; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            elif need_cmd csc "C#"; then
                if ! csc -out:"$outdir/${basename}_cs.exe" -unsafe "$file"; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        rs)
            echo "Rust: $benchmark/$filename"
            if need_cmd rustc "Rust"; then
                if ! rustc -O -C target-cpu=native -C codegen-units=1 "$file" -o "$outdir/${basename}_rust"; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        go)
            echo "Go: $benchmark/$filename"
            if need_cmd go "Go"; then
                if ! go build -o "$outdir/${basename}_go" "$file"; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        java)
            echo "Java: $benchmark/$filename"
            if need_cmd javac "Java"; then
                if ! javac -d "$outdir" "$file"; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        scala)
            echo "Scala: $benchmark/$filename"
            if need_cmd scalac "Scala"; then
                if ! scalac -d "$outdir" "$file"; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        hs)
            echo "Haskell: $benchmark/$filename"
            if need_cmd ghc "Haskell"; then
                if ! ghc -O2 "$file" -o "$outdir/${basename}_hs" -outputdir "$outdir"; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        ml)
            echo "OCaml: $benchmark/$filename"
            if need_cmd ocamlopt "OCaml"; then
                if ! ocamlopt -o "$outdir/${basename}_ocaml" "$file"; then
                    echo "Failed to compile $benchmark/$filename"
                fi
            fi
            ;;

        py|rb|js|lua|php)
            echo "$extension: $benchmark/$filename (copying to output)"
            copy_source "$file" "$outdir"
            ;;

        *)
            echo "Copying unknown file: $filename"
            copy_source "$file" "$outdir"
            ;;

    esac

done

echo "Done."