#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/.." && pwd)"
benchmarks_dir="$repo_root/benchmarks"
bin_dir="$repo_root/bin"

mkdir -p "$bin_dir"

if [[ ! -d "$benchmarks_dir" ]]; then
    echo "Benchmarks directory not found: $benchmarks_dir" >&2
    exit 1
fi

compile_source() {
    local source_file="$1"
    local benchmark_dir="$2"
    local benchmark_name="$3"
    local output_dir="$bin_dir/$benchmark_name"
    local base_name
    local stem
    local extension

    mkdir -p "$output_dir"

    base_name="$(basename "$source_file")"
    stem="${base_name%.*}"
    extension="${base_name##*.}"

    case "$extension" in
        c)
            gcc -O2 -o "$output_dir/$stem" "$source_file"
            ;;
        cc|cpp|cxx)
            g++ -O2 -std=c++17 -o "$output_dir/$stem" "$source_file"
            ;;
        cs)
            if command -v mcs >/dev/null 2>&1; then
                mcs -out:"$output_dir/$stem.exe" "$source_file"
                mv "$output_dir/$stem.exe" "$output_dir/$stem"
            elif command -v csc >/dev/null 2>&1; then
                csc /out:"$output_dir/$stem.exe" "$source_file"
                mv "$output_dir/$stem.exe" "$output_dir/$stem"
            else
                cp "$source_file" "$output_dir/$base_name"
            fi
            ;;
        go)
            if command -v go >/dev/null 2>&1; then
                GO111MODULE=on go build -o "$output_dir/$stem" "$source_file"
            else
                cp "$source_file" "$output_dir/$base_name"
            fi
            ;;
        hs)
            if command -v ghc >/dev/null 2>&1; then
                ghc -O2 -o "$output_dir/$stem" "$source_file"
            else
                cp "$source_file" "$output_dir/$base_name"
            fi
            ;;
        java)
            if command -v javac >/dev/null 2>&1; then
                mkdir -p "$output_dir/classes"
                javac -d "$output_dir/classes" "$source_file"
                cp "$output_dir/classes/$stem.class" "$output_dir/$stem.class"
            else
                cp "$source_file" "$output_dir/$base_name"
            fi
            ;;
        js|lua|ml|php|pl|py|rb|scala|yarv)
            cp "$source_file" "$output_dir/$base_name"
            ;;
        rs)
            if command -v rustc >/dev/null 2>&1; then
                rustc -O -o "$output_dir/$stem" "$source_file"
            else
                cp "$source_file" "$output_dir/$base_name"
            fi
            ;;
        *)
            cp "$source_file" "$output_dir/$base_name"
            ;;
    esac
}

while IFS= read -r benchmark_dir; do
    [[ -d "$benchmark_dir" ]] || continue

    benchmark_name="$(basename "$benchmark_dir")"
    echo "Processing $benchmark_name"

    while IFS= read -r source_file; do
        [[ -f "$source_file" ]] || continue
        compile_source "$source_file" "$benchmark_dir" "$benchmark_name"
    done < <(find "$benchmark_dir" -maxdepth 1 -type f | sort)
done < <(find "$benchmarks_dir" -mindepth 1 -maxdepth 1 -type d | sort)

echo "Compilation complete. Outputs are in $bin_dir"
